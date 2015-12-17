#include "project.hh"

#include "document_lib/cpp_parser.hh"

#include <boost/filesystem.hpp>

#include <stdexcept>
#include <fstream>
#include <regex>
#include <tuple>

namespace cpped {

namespace fs = boost::filesystem;

project::project()
	: index_(0, 0)
{
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

			data.compilation_commands_.clear();
			data.compilation_commands_.reserve(command.size());
			for(unsigned i = 0; i < command.size(); ++i)
			{
				data.compilation_commands_.emplace_back(command.get_arg(i).c_str());
			}
		}
	}
}

document::document& project::open_file(const fs::path& file)
{
	fs::path absolute = fs::absolute(file);
	auto it = open_files_.find(absolute);
	if (it == open_files_.end())
	{
		// TODO find compilation database
		auto doc_ptr = std::make_unique<document::document>();
		doc_ptr->load_from_file(absolute, std::make_unique<cpped::document::cpp_parser>());
		doc_ptr->parse_language();

		auto p = open_files_.insert(std::make_pair(absolute, std::move(doc_ptr)));
		assert(p.second);
		return *p.first->second;
	}
	else
	{
		return *it->second;
	}
}

document::document&project::get_open_file(const boost::filesystem::path& file)
{
	fs::path absolute = fs::absolute(file);
	auto it = open_files_.find(absolute);
	if (it == open_files_.end())
	{
		throw std::runtime_error("No such file");
	}
	return *it->second;
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

project load_cmake_project(const std::string& build_directory)
{
	project pr;
	fs::path build_dir(build_directory);

	if (!fs::is_directory(build_dir))
	{
		throw std::runtime_error("expected CMake build directory: " + build_directory);
	}

	fs::path cmake_cache_path = build_dir / "CMakeCache.txt";
	if (!fs::exists(cmake_cache_path))
	{
		throw std::runtime_error("expected configured CMake build directory: " + build_directory);
	}

	fs::path compile_commands_path = build_dir / "compile_commands.json";
	if (!fs::exists(compile_commands_path))
	{
		throw std::runtime_error("compile_commands.json not found, run cmake with -DCMAKE_EXPORT_COMPILE_COMMANDS=1 or set the vriable in your CMakeLists.txt");
	}

	cmake_info info = parse_cmake_cache(cmake_cache_path.string());

	pr.set_name(info.project_name);
	pr.add_directory(info.source_dir);
	pr.add_compilation_database_file(compile_commands_path.string());

	return pr;
}

}

