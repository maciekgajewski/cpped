#pragma once

#include "document_lib/document.hh"

#include <boost/filesystem.hpp>

#include <vector>
#include <string>
#include <map>

namespace cpped {

namespace backend { class endpoint; }

// Project - a collection of source files, targets, a code model
class project
{
public:
	project(backend::endpoint& ep);

	void open_cmake_project(const boost::filesystem::path& build_dir);
	document::document& open_file(const boost::filesystem::path& file);
	document::document& get_open_file(const boost::filesystem::path& file);

	template<typename OutIt>
	void get_all_project_files(OutIt out) const;
	template<typename OutIt>
	void get_all_open_files(OutIt out) const;

private:

	backend::endpoint& endpoint_;

	// Currently open files
	std::map<boost::filesystem::path, std::unique_ptr<document::document>> open_files_;
};

project load_cmake_project(const std::string& build_directory);


template<typename OutIt>
void project::get_all_project_files(OutIt out) const
{
	// TODO request from backend
}

template<typename OutIt>
void project::get_all_open_files(OutIt out) const
{
	std::transform(open_files_.begin(), open_files_.end(), out,
		[](const auto& p) { return p.first; });
}

}
