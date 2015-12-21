#pragma once

#include "document_lib/document.hh"

#include <boost/filesystem.hpp>

#include <vector>
#include <string>
#include <map>

namespace cpped {

// Project - a collection of source files, targets, a code model
class project
{
public:
	project();

	void set_name(const std::string& n) { name_ = n; }
	const std::string& get_name() const { return name_; }

	document::document& open_file(const boost::filesystem::path& file);
	document::document& get_open_file(const boost::filesystem::path& file);

	template<typename OutIt>
	void get_all_project_files(OutIt out) const;

	template<typename OutIt>
	void get_all_open_files(OutIt out) const;

private:

	// Project name
	std::string name_;

	// All the files which belong to the project. This may include non-cpp files, but
	// will not include system headers etc.
	//
	// Absolute paths
	std::vector<boost::filesystem::path> files_;

	// Currently open files
	std::map<boost::filesystem::path, std::unique_ptr<document::document>> open_files_;
};

project load_cmake_project(const std::string& build_directory);


template<typename OutIt>
void project::get_all_project_files(OutIt out) const
{
	std::copy(files_.begin(), files_.end(), out);
}

template<typename OutIt>
void project::get_all_open_files(OutIt out) const
{
	for(const auto& p : open_files_)
	{
		*out = p.first;
		++out;
	}
}

}
