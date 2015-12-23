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
}


void project::open_cmake_project(const boost::filesystem::path& build_dir)
{
	endpoint_.send_message(backend::messages::open_cmake_project{build_dir});
}

document::document& project::open_file(const fs::path& file)
{
	/* TODO send request to backend
	fs::path absolute = fs::absolute(file);
	auto it = open_files_.find(absolute);
	if (it == open_files_.end())
	{
		// TODO find compilation database
		std::unique_ptr<document::iparser> parser;
		file_data& data = get_file_data(absolute);
		if (data.type_ == file_type::cpp)
		{
			if (data.translation_unit_.is_null())
			{
				parse_file(absolute, data);
			}
			parser = std::make_unique<document::cpp_parser>(data.translation_unit_);
		}

		auto doc_ptr = std::make_unique<document::document>();
		doc_ptr->load_from_file(absolute, std::move(parser));
		doc_ptr->parse_language(); // TODO optimize, no need to parse twice

		auto p = open_files_.insert(std::make_pair(absolute, std::move(doc_ptr)));
		assert(p.second);
		return *p.first->second;
	}
	else
	{
		return *it->second;
	}
	*/
	return get_open_file(file);
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

}

