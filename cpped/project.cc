#include "project.hh"

#include "ipc_lib/endpoint.hh"

#include "backend_lib/messages.hh"

#include "nct/status_message.hh"

#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>

#include <stdexcept>
#include <tuple>
#include <sstream>

namespace cpped {

namespace fs = boost::filesystem;

project::project(ipc::endpoint& ep)
	: endpoint_(ep)
{
	endpoint_.register_message_handler<backend::messages::file_tokens_feed>(
		[this](const backend::messages::file_tokens_feed& tf) { on_file_tokens(tf); });

	endpoint_.register_message_handler<backend::messages::status_feed>(
		[this](const backend::messages::status_feed& sf) { status_signal(sf.message); });
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

edited_file& project::open_file(const fs::path& path)
{
	fs::path absolute = fs::absolute(path);

	edited_file* file = find_file(absolute);

	if (!file)
	{
		nct::status_provider sp("Opening file...");

		// request file from backend
		backend::messages::open_file_request request{absolute};
		backend::messages::open_file_reply reply;
		endpoint_.send_sync_request(request, reply);

		if (!reply.error.empty())
		{
			throw std::runtime_error("Failed to open file: " + reply.error);
		}

		document::document doc;
		doc.load_from_raw_data(reply.data, absolute, reply.tokens, reply.new_file);

		open_files_.push_front(
			std::make_unique<edited_file>(endpoint_, std::move(doc), reply.parsed, reply.new_file)
			);

		return *open_files_.front();
	}
	else
	{
		return *file;
	}
}

edited_file& project::get_open_file(const boost::filesystem::path& path)
{
	fs::path absolute = fs::absolute(path);

	edited_file* file = find_file(absolute);

	if (!file)
	{
		throw std::runtime_error("No such file");
	}
	return *file;
}

std::unique_ptr<edited_file> project::make_unsaved_file()
{
	return std::make_unique<edited_file>(endpoint_, document::document{}, false, true);
}

void project::on_file_tokens(const backend::messages::file_tokens_feed& token_feed)
{
	edited_file* file = find_file(token_feed.path);

	if (file)
	{
		file->on_file_tokens(token_feed);
	}
}

edited_file* project::find_file(const boost::filesystem::path& path)
{
	auto it = boost::find_if(
		open_files_, [&](const auto& ef_ptr)
		{
			return ef_ptr->get_path() == path;
		});

	if (it == open_files_.end())
	{
		return nullptr;
	}
	else
	{
		return it->get();
	}
}

}

