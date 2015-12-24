#pragma once

#include "compilation_unit.hh"

#include "clang_lib/clang.hh"

#include <map>

namespace cpped { namespace backend {

class event_dispatcher;

class project
{
public:

	project(event_dispatcher& ed);

	// IPC message habdlers

	void open_cmake_project(const boost::filesystem::path& build_directory);
	void open_file(const boost::filesystem::path& path);

private:

	enum class file_type
	{
		cpp,
		other
	};

	struct file_data
	{
		file_type type_ = file_type::other;
	};

	file_data& get_or_create_file_data(const boost::filesystem::path& path);

	compilation_unit& get_or_create_unit(const boost::filesystem::path& path);
	compilation_unit* get_unit(const boost::filesystem::path& path) const;

	// Database of all information about a file that we know
	std::map<boost::filesystem::path, std::unique_ptr<file_data>> file_data_;

	// adds all files in the directory to the project
	void add_directory(const boost::filesystem::path& source_dir);
	void add_compilation_database_file(const boost::filesystem::path& comp_database_path);


	void scheduled_parse_file(const boost::filesystem::path& path);

	std::string name_;

	// All the files which belong to the project. This may include non-cpp files, but
	// will not include system headers etc.
	//
	// Absolute paths
	std::vector<boost::filesystem::path> files_;

	// All the compilation units know by the project
	std::map<boost::filesystem::path, std::unique_ptr<compilation_unit>> units_;

	event_dispatcher& event_dispatcher_;
	clang::index index_;
};

}}
