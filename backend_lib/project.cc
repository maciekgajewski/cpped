#include "project.hh"

#include "event_dispatcher.hh"
#include "messages.hh"

#include "clang_lib/flags.hh"

#include <boost/filesystem.hpp>

#include <fstream>
#include <regex>

namespace cpped { namespace backend {

namespace fs = boost::filesystem;

project::project(event_dispatcher& ed)
	: event_dispatcher_(ed), index_(0, 0)
{
	// register message-receiving functions

	event_dispatcher_.register_message_handler<messages::open_cmake_project>(
		[this](const messages::open_cmake_project& msg) { this->open_cmake_project(msg.build_dir); });
	event_dispatcher_.register_message_handler<messages::open_file_request>(
		[this](const messages::open_file_request& msg) { this->open_file(msg.file); });
}

void project::add_directory(const boost::filesystem::path& dir_path)
{
	for(fs::recursive_directory_iterator it(dir_path); it != fs::recursive_directory_iterator(); ++it)
	{
		const fs::directory_entry& entry = *it;

		// skip hidden directories
		if (entry.status().type() == fs::directory_file && entry.path().filename().string()[0] == '.')
		{
			it.no_push();
		}
		// add file
		if (entry.status().type() == fs::regular_file)
		{
			files_.push_back(entry.path());
		}
	}
}

void project::add_compilation_database_file(const fs::path& comp_database_path)
{
	fs::path directory = fs::absolute(comp_database_path).parent_path();
	clang::compilation_database db(directory);

	for(const fs::path& file : files_)
	{
		clang::compile_commands cc = db.get_compile_commands_for_file(file);

		if (!cc.is_null() && cc.size() > 0)
		{
			clang::compile_command command = cc.get_command(0);
			file_data& data = get_file_data(file);
			data.compilation_commands_ = clang::get_sanitized_flags(command, file);
			data.type_ = file_type::cpp; // if clangs knows how to compile it, it must be it

			// schedule file parsing
			event_dispatcher_.schedule_job([this, file]() { parse_file(file); });
		}
	}
}
project::file_data&project::get_file_data(const boost::filesystem::path& file)
{
	assert(file.is_absolute());
	std::unique_ptr<file_data>& p = file_data_[file];
	if (!p)
	{
		p = std::make_unique<file_data>();
	}

	return *p;
}

void project::parse_file(const fs::path& path)
{
	file_data& data = get_file_data(path);

	assert(data.type_ == file_type::cpp);
	assert(data.translation_unit_.is_null());
	assert(path.is_absolute());

	// if there exists a provisonal TU, dispose of it
	if (!data.provisional_translation_unit_.is_null())
	{
		data.provisional_translation_unit_.dispose();
	}

	std::vector<const char*> cmdline;
	cmdline.reserve(data.compilation_commands_.size()+1);
	assert(data.compilation_commands_.size() > 1);

	std::transform(
		data.compilation_commands_.begin(), data.compilation_commands_.end(),
		std::back_inserter(cmdline),
		[&](const std::string& c) { return c.c_str(); });

	data.translation_unit_.parse(
		index_,
		path.string().c_str(),
		nullptr, 0, // unsaved data - none yet
		cmdline);
}

struct cmake_info
{
	std::string source_dir;
	std::string project_name;
};

static cmake_info parse_cmake_cache(const std::string& cmake_cache_path)
{
	std::ifstream file(cmake_cache_path);
	cmake_info info;

	// find project name
	std::regex project_name_rx("CMAKE_PROJECT_NAME:STATIC=(.+)");

	for (std::string line; std::getline(file, line); )
	{
		std::match_results<std::string::const_iterator> match;
		if (std::regex_match(line, match, project_name_rx))
		{
			info.project_name = match[1].str();
			break;
		}
	}
	if (info.project_name.empty())
	{
		throw std::runtime_error("Unable to find CMAKE_PROJECT_NAME in CMakeCache.txt");
	}

	// find source directory
	std::regex sourcedir_rx(info.project_name + "_SOURCE_DIR:STATIC=(.+)");
	for (std::string line; std::getline(file, line); )
	{
		std::match_results<std::string::const_iterator> match;
		if (std::regex_match(line, match, sourcedir_rx))
		{
			info.source_dir = match[1].str();
			break;
		}
	}
	if (info.source_dir.empty())
	{
		throw std::runtime_error("Unable to find <project>_SOURCE_DIR in CMakeCache.txt");
	}

	return info;
}

void project::open_cmake_project(const boost::filesystem::path& build_directory)
{
	fs::path build_dir(build_directory);

	if (!fs::is_directory(build_dir))
	{
		throw std::runtime_error("expected CMake build directory: " + build_directory.string());
	}

	fs::path cmake_cache_path = build_dir / "CMakeCache.txt";
	if (!fs::exists(cmake_cache_path))
	{
		throw std::runtime_error("expected configured CMake build directory: " + build_directory.string());
	}

	fs::path compile_commands_path = build_dir / "compile_commands.json";
	if (!fs::exists(compile_commands_path))
	{
		throw std::runtime_error("compile_commands.json not found, run cmake with -DCMAKE_EXPORT_COMPILE_COMMANDS=1 or set the vriable in your CMakeLists.txt");
	}

	cmake_info info = parse_cmake_cache(cmake_cache_path.string());

	name_ = info.project_name;
	add_directory(info.source_dir);
	add_compilation_database_file(compile_commands_path.string());
}

void project::open_file(const boost::filesystem::path& path)
{
	// TODO
}


}}
