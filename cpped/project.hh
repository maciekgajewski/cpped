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

	// adds all files in the directory to the project
	void add_directory(const boost::filesystem::path& source_dir);
	void add_compilation_database_file(const boost::filesystem::path& comp_database_path);

	void set_name(const std::string& n) { name_ = n; }
	const std::string& get_name() const { return name_; }

	document::document& open_file(const boost::filesystem::path& file);
	document::document& get_open_file(const boost::filesystem::path& file);

private:

	std::string name_;
	std::vector<boost::filesystem::path> files_;
	std::map<boost::filesystem::path, std::unique_ptr<document::document>> open_files_;


};

project load_cmake_project(const std::string& build_directory);

}
