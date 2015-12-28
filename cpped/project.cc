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
		// request file from backend
		backend::messages::open_file_request request{absolute};
		backend::messages::open_file_reply reply;
		endpoint_.send_sync_request(request, reply);

		auto doc_ptr = std::make_unique<document::document>();
		doc_ptr->load_from_raw_data(reply.data, absolute, reply.tokens.tokens);

		emit_parsing_status(reply.tokens);

		doc_ptr->document_changed_signal.connect(
			[this, d=doc_ptr.get()] () { on_document_changed(*d); });

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

void project::request_parsing(const document::document& doc)
{
	backend::messages::document_changed_feed feed;
	feed.file = doc.get_file_name();
	feed.version = doc.get_current_version();
	feed.data.assign(doc.get_raw_data().begin(), doc.get_raw_data().end());

	endpoint_.send_message(feed);
}

void project::on_document_changed(const document::document& doc)
{
	if (!parsing_in_progress_)
	{
		request_parsing(doc);
		parsing_in_progress_ = true;
	}
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
	auto needs_parsing = std::find_if(open_files_.begin(), open_files_.end(),
		[&](const auto& pair)
		{
			return pair.second.last_version_parsed < pair.second.document->get_current_version();
		});
	if (needs_parsing != open_files_.end())
	{
		request_parsing(*needs_parsing->second.document);
		parsing_in_progress_ = true;
	}
}

}

