#pragma once

#include <vector>
#include <string>

namespace cpped {

// Project - a collection of source files, targets, a code model
class project
{
public:
	project();

	// adds all files in the directory to the project
	void add_directory(const std::string& source_dir);
	void add_compilation_database_file(const std::string& comp_database_path);

	void set_name(const std::string& n) { name_ = n; }
	const std::string& get_name() const { return name_; }

private:

	std::string name_;
	std::vector<std::string> files_;
};

project load_cmake_project(const std::string& build_directory);

}
