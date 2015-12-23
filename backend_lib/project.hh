#pragma once

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
		std::vector<std::string> compilation_commands_;
		file_type type_ = file_type::other;
		clang::translation_unit translation_unit_;

		// Translation Unit for files that are not part of the project
		clang::translation_unit provisional_translation_unit_;
	};

	file_data& get_file_data(const boost::filesystem::path& file);
	void parse_file(const boost::filesystem::path& path);

	// Database of all information about a file that we know
	std::map<boost::filesystem::path, std::unique_ptr<file_data>> file_data_;

	// adds all files in the directory to the project
	void add_directory(const boost::filesystem::path& source_dir);
	void add_compilation_database_file(const boost::filesystem::path& comp_database_path);

	std::string name_;

	// All the files which belong to the project. This may include non-cpp files, but
	// will not include system headers etc.
	//
	// Absolute paths
	std::vector<boost::filesystem::path> files_;

	//std::map<boost::filesystem::path,

	event_dispatcher& event_dispatcher_;
	clang::index index_;
};

}}
