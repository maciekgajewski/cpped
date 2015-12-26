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

	event_dispatcher_.register_message_handler<messages::open_cmake_project_request>(
		[this](const messages::open_cmake_project_request& msg)
		{
			try
			{
				open_cmake_project(msg.build_dir);
				event_dispatcher_.send_message(messages::open_cmake_project_reply{{}, files_});
			}
			catch(const std::exception& e)
			{
				event_dispatcher_.send_message(messages::open_cmake_project_reply{e.what(), {}});
			}
		});

	event_dispatcher_.register_message_handler<messages::open_file_request>(
		[this](const messages::open_file_request& request)
		{
			try
			{
				open_file& file = this->open(request.file);

				messages::open_file_reply reply;
				reply.file = request.file;
				reply.data.assign(file.get_data().begin(), file.get_data().end());
				event_dispatcher_.send_message(reply);

				// get tokens now
				messages::file_tokens_feed tokens_feed;
				tokens_feed.file = request.file;
				tokens_feed.version = 0;
				tokens_feed.tokens = file.parse(get_unsaved_data());
				if (!tokens_feed.tokens.empty())
				{
					event_dispatcher_.send_message(tokens_feed);
				}
			}
			catch(const std::exception& e)
			{
				event_dispatcher_.send_message(messages::open_file_reply{request.file, e.what(), {}});
			}
		});
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

	for(const fs::path& path : files_)
	{
		clang::compile_commands cc = db.get_compile_commands_for_file(path);

		if (!cc.is_null() && cc.size() > 0)
		{
			file_data& data = get_or_create_file_data(path);
			data.type_ = file_type::cpp; // if clangs knows how to compile it, it must be it

			compilation_unit& u = get_or_create_unit(path);
			clang::compile_command command = cc.get_command(0);
			u.set_compilation_flags(clang::get_sanitized_flags(command, path));

			// schedule file parsing
			event_dispatcher_.schedule_job([this, path]() { scheduled_parse_file(path); });
		}
	}
}

void project::scheduled_parse_file(const boost::filesystem::path& path)
{
	compilation_unit* unit = get_unit(path);
	if (unit && !unit->needs_parsing())
	{
		LOG("Scheduled parsing of " << path);
		unit->parse(get_unsaved_data());
	}
}

template<typename Container, typename Value>
static bool contains(const Container& c, const Value& v)
{
	return std::find(std::begin(c), std::end(c), v) != std::end(c);
}

project::file_type project::get_type_by_extensions(const boost::filesystem::path& path)
{
	// known cpp extensions
	static const char* cpp_ext[] = {".cc", ".cpp", ".cxx", ".C"};
	static const char* c_ext[] = {".c"};
	static const char* h_ext[] = {".h", ".hh", ".H", ".hxx", ".hpp", ".ipp", ".ixx", ""};

	if (contains(cpp_ext, path.extension()))
		return file_type::cpp;
	else if (contains(c_ext, path.extension()))
		return file_type::c;
	else if (contains(h_ext, path.extension()))
		return file_type::header;
	else
		return file_type::other;
}

std::vector<std::string> project::get_default_flags(const boost::filesystem::path& path)
{
	std::vector<std::string> flags;
	flags.reserve(3);
	flags.push_back("-fdiagnostics-color=never");
	flags.push_back("-x");

	file_type type = get_type_by_extensions(path);
	switch(type)
	{
		case file_type::c:
			flags.push_back("c");
			break;
		case file_type::cpp:
			flags.push_back("c++");
			break;
		case file_type::header:
		case file_type::other:
			flags.push_back("c++-header");
			break;
	};

	return flags;
}

std::vector<std::string> project::get_flags_for_path(const boost::filesystem::path& path) const
{
	// the heuristics is the foolowing:
	// if there is another CU in the same directory, use it's flags
	// otherwise, use default set

	auto it = std::find_if(units_.begin(), units_.end(),
		[&](const auto& pair)
		{
			const auto& p = pair.first;
			return path.parent_path() == p.parent_path();
		});

	if (it != units_.end())
	{
		assert(it->first != path);
		LOG("For file " << path << " using the same flags as for " << it->first);
		return it->second->get_compilation_flags();
	}

	LOG("For file " << path << " using the default flags");
	return get_default_flags(path);
}

compilation_unit* project::get_or_create_unit_for_file(const boost::filesystem::path& path)
{
	// check if the file already has a compilation unit
	auto it = units_.find(path);
	if (it != units_.end())
	{
		LOG("Found already existing compiltion unit for " << path);
		return it->second.get();
	}

	// check if the file is a C/C++ file, andf should have CU created
	file_type type = get_type_by_extensions(path);
	if (type == file_type::cpp || type == file_type::c)
	{
		LOG("File " << path << " idnetified as C++, creating compilation unit");
		file_data& data = get_or_create_file_data(path);
		data.type_ = file_type::cpp;

		std::unique_ptr<compilation_unit> u = std::make_unique<compilation_unit>(path, index_);
		std::vector<std::string> flags = get_flags_for_path(path);
		u->set_compilation_flags(flags);

		compilation_unit* ptr = u.get();
		units_[path] = std::move(u);
		return ptr;
	}

	LOG("Unable to create compilation unit for file " << path);
	// TODO: look for included files
	return nullptr;
}

std::vector<CXUnsavedFile> project::get_unsaved_data()
{
	std::vector<CXUnsavedFile> unsaved_data;
	unsaved_data.reserve(open_files_.size()); // upper bound

	for(const auto& p : open_files_)
	{
		const open_file& file = *p.second;
		const fs::path& path = p.first;

		if (file.has_unsaved_data())
		{
			unsaved_data.push_back(
				CXUnsavedFile{
					path.c_str(),
					file.get_data().data(),
					file.get_data().size()});
		}
	}
	return unsaved_data;
}

project::file_data&project::get_or_create_file_data(const boost::filesystem::path& path)
{
	assert(path.is_absolute());
	std::unique_ptr<file_data>& p = file_data_[path];
	if (!p)
	{
		p = std::make_unique<file_data>();
	}

	return *p;
}

compilation_unit& project::get_or_create_unit(const boost::filesystem::path& path)
{
	assert(path.is_absolute());
	std::unique_ptr<compilation_unit>& p = units_[path];
	if (!p)
	{
		p = std::make_unique<compilation_unit>(path, index_);
	}

	return *p;
}

compilation_unit* project::get_unit(const boost::filesystem::path& path) const
{
	assert(path.is_absolute());
	auto it = units_.find(path);
	if (it == units_.end())
	{
		return nullptr;
	}
	else
	{
		return it->second.get();
	}
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
	LOG("Opening cmake project at " << build_directory);

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
	LOG("Opening CMake project, name: " << name_ << ", source dir=" << info.source_dir);
	add_directory(info.source_dir);
	add_compilation_database_file(compile_commands_path.string());
}

open_file& project::open(const boost::filesystem::path& path)
{
	assert(path.is_absolute());
	assert(open_files_.find(path) == open_files_.end());

	LOG("Opening file " << path);

	auto file_up = std::make_unique<open_file>(path);
	open_file* file = file_up.get();
	open_files_[path] = std::move(file_up);

	compilation_unit* cu = get_or_create_unit_for_file(path);
	if (cu)
	{
		file->set_compilation_unit(cu);
	}
	else
	{
		// TODO create provisional compilation unit
	}

	return *file;
}


}}
