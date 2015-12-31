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
			LOG("Recevied file open request: " << request.file);
			try
			{
				open_file& file = this->open(request.file);

				messages::open_file_reply reply;
				reply.file = request.file;
				reply.data.assign(file.get_data().begin(), file.get_data().end());
				auto state = file.parse(get_unsaved_data());
				reply.tokens = std::move(state);
				LOG("Sending back file data");
				event_dispatcher_.send_message(reply);
			}
			catch(const std::exception& e)
			{
				LOG("sending back an error: " << e.what());
				event_dispatcher_.send_message(messages::open_file_reply{request.file, e.what()});
			}
		});

	event_dispatcher_.register_message_handler<messages::document_changed_feed>(
		[this](const messages::document_changed_feed& feed)
		{
			LOG("Data feed recevied for file " << feed.file << ", version " << feed.version);
			auto it = open_files_.find(feed.file);
			if (it != open_files_.end())
			{
				open_file& file = *it->second;
				file.set_data(feed.data, feed.version);
				touch_units(feed.file);
				// get completion (if requested)
				if (feed.cursor_position)
				{
					LOG("Data feed requests completion, obtaining");
					messages::complete_at_reply reply;
					reply.results = file.complete_at(get_unsaved_data(), *feed.cursor_position);
					event_dispatcher_.send_message(reply);
				}
				// reparse
				messages::file_tokens_feed tokens_feed;
				tokens_feed.file = feed.file;
				tokens_feed.version = feed.version;
				auto state = file.parse(get_unsaved_data());
				tokens_feed.tokens = std::move(state);
				if (!tokens_feed.tokens.tokens.empty())
				{
					event_dispatcher_.send_message(tokens_feed);
				}
				LOG("Data feed processed");
			}
			else
			{
				LOG("FATAL: Data feed recevied for unknown file: " << feed.file);
				// inconsistent data
				std::terminate();
			}
		});

	event_dispatcher_.register_message_handler<messages::complete_at_request>(
		[this](const messages::complete_at_request& request)
		{
			messages::complete_at_reply reply;

			auto it = open_files_.find(request.file);
			if (it != open_files_.end())
			{
				open_file& file = *it->second;
				reply.results = file.complete_at(get_unsaved_data(), request.cursor_position);
			}

			event_dispatcher_.send_message(reply);
		});
}

void project::touch_units(const fs::path& changed_file)
{
	assert(changed_file.is_absolute());

	auto it = units_.find(changed_file);
	if (it != units_.end())
	{
		// the file is a CU, mark it as dirty
		it->second->mark_dirty();
	}
	else
	{
		for(const auto& pair : units_)
		{
			if (pair.second->includes(changed_file))
			{
				pair.second->mark_dirty();
			}
		}
	}
}

void project::add_directory(const fs::path& dir_path)
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

	files_to_parse_ = 0;
	files_parsed_ = 0;
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
			files_to_parse_++;
		}
	}
}

void project::scheduled_parse_file(const boost::filesystem::path& path)
{
	compilation_unit* unit = get_unit(path);
	if (unit && unit->needs_parsing())
	{
		LOG("Scheduled parsing of " << path);
		unit->parse(get_unsaved_data());
		files_parsed_++;

		if (files_parsed_ < files_to_parse_)
		{
			event_dispatcher_.send_message(messages::status_feed{
				"Parsing files: " + std::to_string(files_parsed_) + "/" + std::to_string(files_to_parse_)});
		}
		else
		{
			event_dispatcher_.send_message(messages::status_feed{});
		}
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

	clang::get_common_flags(flags);

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
	flags.push_back("-std=c++14");

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

std::unique_ptr<compilation_unit> project::make_compilation_unit(const fs::path& path)
{
	std::unique_ptr<compilation_unit> u = std::make_unique<compilation_unit>(path, index_);
	std::vector<std::string> flags = get_flags_for_path(path);
	u->set_compilation_flags(flags);

	return u;
}

compilation_unit* project::find_unit_for_header(const fs::path& path) const
{
	auto it = std::find_if(units_.begin(), units_.end(),
		[&](const auto& pair)
		{
			return pair.second->includes(path);
		});

	if (it != units_.end())
	{
		LOG("Found compilation unit for include " << path << " : " << it->first);
		return it->second.get();
	}
	else
	{
		LOG("Failed to find compilation unit including " << path);
		return nullptr;
	}
}

void project::get_or_create_unit_for_file(open_file& file)
{
	// check if the file already has a compilation unit
	auto it = units_.find(file.get_path());
	if (it != units_.end())
	{
		LOG("Found already existing compiltion unit for " << file.get_path());
		file.set_compilation_unit(it->second.get());
		return;
	}

	// check if the file is a C/C++ file, andf should have CU created
	file_type type = get_type_by_extensions(file.get_path());
	if (type == file_type::cpp || type == file_type::c)
	{
		LOG("File " << file.get_path() << " idnetified as C++, creating compilation unit");
		file_data& data = get_or_create_file_data(file.get_path());
		data.type_ = file_type::cpp;

		std::unique_ptr<compilation_unit> u = make_compilation_unit(file.get_path());
		file.set_compilation_unit(u.get());
		u->includes_changed_signal.connect([this, uptr=u.get()]() { on_includes_updated(*uptr); });
		units_[file.get_path()] = std::move(u);
	}
	else if (type == file_type::header)
	{
		file_data& data = get_or_create_file_data(file.get_path());
		data.type_ = file_type::header;

		compilation_unit* unit = find_unit_for_header(file.get_path());
		if (unit)
		{
			LOG("File " << file.get_path() << " idnetified as C++ header, using unit for " << unit->get_path());
			file.set_compilation_unit(unit);
		}
		else
		{
			LOG("File " << file.get_path() << " idnetified as C++ header, creating provisional compilation unit");

			std::unique_ptr<compilation_unit> u = make_compilation_unit(file.get_path());
			file.set_provisional_compilation_unit(std::move(u));
		}
	}
	else
	{
		LOG("Unable to create compilation unit for file " << file.get_path());
	}
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

void project::on_includes_updated(compilation_unit& cu)
{
	// go trough open files, and if any of them is using provisional CU, try replacing it with the new one

	LOG("Includes for unit " << cu.get_path() << " changed");

	for(const auto& pair : open_files_)
	{
		if (pair.second->uses_provisional_unit() && cu.includes(pair.first))
		{
			pair.second->set_compilation_unit(&cu);
			LOG("Replacing provisional cu on " << pair.first << " with " << cu.get_path());
			messages::file_tokens_feed feed;
			feed.file = pair.first;
			auto state = pair.second->parse(get_unsaved_data());
			feed.tokens = std::move(state);
			feed.version = pair.second->get_version();

			event_dispatcher_.send_message(feed);
		}
	}
}

project::file_data& project::get_or_create_file_data(const boost::filesystem::path& path)
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
		p->includes_changed_signal.connect([this, uptr=p.get()]() { on_includes_updated(*uptr); });
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

	get_or_create_unit_for_file(*file);

	return *file;
}


}}
