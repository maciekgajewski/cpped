#include "project.hh"

#include "backend_lib/endpoint.hh"
#include "backend_lib/messages.hh"

#include <boost/filesystem.hpp>

#include <stdexcept>
#include <tuple>

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
		doc_ptr->load_from_raw_data(reply.data, file);

		auto p = open_files_.insert(std::make_pair(absolute, std::move(doc_ptr)));
		assert(p.second);
		return *p.first->second;
	}
	else
	{
		return *it->second;
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
	return *it->second;
}

void project::on_file_tokens(const backend::messages::file_tokens_feed& token_feed)
{
	auto it = open_files_.find(token_feed.file);
	if (it != open_files_.end())
	{
		it->second->set_tokens(token_feed.version, token_feed.tokens);
	}
}

}

