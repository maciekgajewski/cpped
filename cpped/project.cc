#include "project.hh"

#include "backend_lib/endpoint.hh"
#include "backend_lib/messages.hh"

#include <boost/filesystem.hpp>

#include <stdexcept>
#include <tuple>
#include <sstream>

namespace cpped {

namespace fs = boost::filesystem;

project::project(backend::endpoint& ep)
	: endpoint_(ep)
{
	endpoint_.register_message_handler<backend::messages::file_tokens_feed>(
		[this](const backend::messages::file_tokens_feed& tf) { on_file_tokens(tf); });
}


void project::open_cmake_project(const boost::filesystem::path& build_dir)
{
	backend::messages::open_cmake_project_reply reply;
	endpoint_.send_sync_request(backend::messages::open_cmake_project_request{build_dir}, reply);

	if (!reply.error.empty())
	{
		throw std::runtime_error(reply.error);
	}

	files_ = std::move(reply.files);
}

document::document& project::open_file(const fs::path& file)
{
	fs::path absolute = fs::absolute(file);
	auto it = open_files_.find(absolute);
	if (it == open_files_.end())
	{
		status_signal("Opening file...");

		// request file from backend
		backend::messages::open_file_request request{absolute};
		backend::messages::open_file_reply reply;
		endpoint_.send_sync_request(request, reply);

		if (!reply.error.empty())
		{
			throw std::runtime_error("Failed to open file: " + reply.error);
		}

		auto doc_ptr = std::make_unique<document::document>();
		doc_ptr->load_from_raw_data(reply.data, absolute, reply.tokens.tokens);

		emit_parsing_status(reply.tokens);

		auto p = open_files_.insert(std::make_pair(absolute, open_file_data{std::move(doc_ptr), 0} ));
		assert(p.second);
		return *p.first->second.document;
	}
	else
	{
		return *it->second.document;
	}
}

document::document& project::get_open_file(const boost::filesystem::path& file)
{
	fs::path absolute = fs::absolute(file);
	auto it = open_files_.find(absolute);
	if (it == open_files_.end())
	{
		throw std::runtime_error("No such file");
	}
	return *it->second.document;
}

void project::send_parse_request(
	const document::document& doc,
	const boost::optional<document::document_position>& cursor_pos)
{
		backend::messages::document_changed_feed feed;
		feed.file = doc.get_file_name();
		feed.version = doc.get_current_version();
		feed.data.assign(doc.get_raw_data().begin(), doc.get_raw_data().end());
		feed.cursor_position = cursor_pos;

		endpoint_.send_message(feed);
}

void project::request_parsing(
	const document::document& doc,
	const boost::optional<document::document_position>& cursor_pos)
{
	if (!parsing_in_progress_)
	{
		send_parse_request(doc, cursor_pos);
		parsing_in_progress_ = true;
	}
	else
	{
		outstanding_parsing_requests_[doc.get_file_name()] = cursor_pos;
	}
}

std::vector<backend::messages::completion_record> project::get_completion(const document::document& doc, const document::document_position& cursor_pos)
{
	status_signal("Getting completion...");

	backend::messages::complete_at_request request;
	request.file = doc.get_file_name();
	request.cursor_position = cursor_pos;

	backend::messages::complete_at_reply reply;

	endpoint_.send_sync_request(request, reply);

	status_signal("");

	return reply.results;
}

void project::emit_parsing_status(const backend::token_data& data) const
{
	// send signal status
	std::ostringstream ss;
	ss << "File parsed, errors: " << data.errors << ", warnings: " << data.warnings;
	status_signal(ss.str());
}

void project::on_file_tokens(const backend::messages::file_tokens_feed& token_feed)
{
	auto it = open_files_.find(token_feed.file);
	if (it != open_files_.end())
	{
		it->second.document->set_tokens(token_feed.version, token_feed.tokens.tokens);
		it->second.last_version_parsed = token_feed.version;

		emit_parsing_status(token_feed.tokens);
	}
	parsing_in_progress_ = false;

	// look for documents needing parsing
	while(!outstanding_parsing_requests_.empty())
	{
		auto it = outstanding_parsing_requests_.begin();
		const fs::path& path = it->first;
		auto open_it = open_files_.find(path);
		if (open_it != open_files_.end())
		{
			const document::document& doc = *open_it->second.document;
			send_parse_request(doc, it->second);
			outstanding_parsing_requests_.erase(it);
			break;
		}
		else
		{
			outstanding_parsing_requests_.erase(it);
		}
	}
}

}

