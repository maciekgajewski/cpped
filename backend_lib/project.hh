#pragma once

#include "compilation_unit.hh"
#include "open_file.hh"

#include "clang_lib/clang.hh"

#include <map>

namespace cpped { namespace backend {

class event_dispatcher;

class project
{
public:

	project(event_dispatcher& ed);

	// IPC message handlers

	void open_cmake_project(const boost::filesystem::path& build_directory);
	open_file& open(const boost::filesystem::path& path);

private:

	enum class file_type
	{
		cpp,
		c,
		header,
		other
	};

	struct file_data
	{
		file_type type_ = file_type::other;
	};

	file_data& get_or_create_file_data(const boost::filesystem::path& path);

	compilation_unit& get_or_create_unit(const boost::filesystem::path& path);
	compilation_unit* get_unit(const boost::filesystem::path& path) const;

	static file_type get_type_by_extensions(const boost::filesystem::path& path);

	// adds all files in the directory to the project
	void add_directory(const boost::filesystem::path& source_dir);
	void add_compilation_database_file(const boost::filesystem::path& comp_database_path);

	void scheduled_parse_file(const boost::filesystem::path& path);

	// Looks for compilation unit containing the file
	void get_or_create_unit_for_file(open_file& file);

	compilation_unit* find_unit_for_header(const boost::filesystem::path& path) const;

	std::unique_ptr<compilation_unit> make_compilation_unit(const boost::filesystem::path& path);

	// Uses heuristics to figure-out the best set of flags for the file
	std::vector<std::string> get_flags_for_path(const boost::filesystem::path& path) const;

	// Returns default set of flags for file
	static std::vector<std::string> get_default_flags(const boost::filesystem::path& path);

	std::vector<CXUnsavedFile> get_unsaved_data();

	// Called when one of the compilation units updates includes
	void on_includes_updated(compilation_unit& cu);

	// touich all CUs containing the file
	void touch_units(const boost::filesystem::path& changed_file);

	// Database of all information about a file that we know
	std::map<boost::filesystem::path, std::unique_ptr<file_data>> file_data_;

	std::string name_;

	// All the files which belong to the project. This may include non-cpp files, but
	// will not include system headers etc.
	//
	// Absolute paths
	std::vector<boost::filesystem::path> files_;

	// All the compilation units know by the project
	std::map<boost::filesystem::path, std::unique_ptr<compilation_unit>> units_;

	// All the currently open files
	std::map<boost::filesystem::path, std::unique_ptr<open_file>> open_files_;

	event_dispatcher& event_dispatcher_;
	clang::index index_;

	// parsing status
	unsigned files_to_parse_ = 0;
	unsigned files_parsed_ = 0;

};

}}
