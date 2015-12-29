#include "flags.hh"

#include "../backend_lib/log.hh"

#include <map>

#include <stdio.h>

// What I'm doing here is (mostly) based on what YCMD is doing

namespace cpped { namespace clang {

namespace fs = boost::filesystem;

static std::vector<fs::path> get_paths_for_compiler(const std::string& compiler)
{

	struct FILE_pcloser
	{
		void operator()(FILE* p) { ::pclose(p); }
	};
	using FILE_ptr = std::unique_ptr<FILE, FILE_pcloser>;

	std::vector<fs::path> paths;

	// the following command works on both gcc and clang, and porduces compatible output
	std::string command = compiler + " -xc++ -E -v /dev/null 2>&1 > /dev/null";

	LOG("Trying to get built-in paths for compiler " << compiler << " using the following command: " << command);


	FILE_ptr p(::popen(command.c_str(), "r"), FILE_pcloser{});
	if (!p)
	{
		LOG(".. failed");
		return paths;
	}

	// reading operation
	static const unsigned BUFSIZE = 1024;
	char buffer[BUFSIZE];

	paths.reserve(10);

	// wait for start market
	bool begin_marker_found = false;
	while(::fgets(buffer, BUFSIZE, p.get()))
	{
		LOG("read: " << buffer);
		if (begin_marker_found)
		{
			// check for end marker
			if (std::strcmp(buffer, "End of search list.\n") == 0)
			{
				// we're done
				break;
			}
			else
			{
				// skip leading whiespace
				std::string s = buffer;
				auto pos = s.find_first_not_of(" \t");
				if (pos != std::string::npos)
				{
					s.assign(s.begin() + pos, s.end()-1);
				}

				LOG(" path found: '" << buffer << "', stripped=" << s);

				fs::path p = fs::canonical(s);
				paths.push_back(p);
			}
		}
		if (std::strcmp(buffer, "#include <...> search starts here:\n") == 0)
		{
			begin_marker_found = true;
		}
	}

	LOG(" ... finished");
	return paths;
}

void get_common_flags(const std::string& compiler, std::vector<std::string>& out)
{
	// inicolude paths per compiler
	static std::map<std::string, std::vector<fs::path>> include_paths_cache;

	auto it = include_paths_cache.find(compiler);
	if (it == include_paths_cache.end())
	{
		std::vector<fs::path> paths = get_paths_for_compiler(compiler);
		if (!paths.empty())
		{
			// store in cache
			auto p = include_paths_cache.insert({compiler, paths});
			it = p.first;
		}
		else
		{
			// try using whatever we've got
			it = include_paths_cache.begin();
		}
	}

	// common required flags
	out.push_back("-fdiagnostics-color=never");

	// system includes
	if (it != include_paths_cache.end())
	{
		LOG("Requested common flags for compiler " << compiler << ", will use flags for " << it->first);
		for(const fs::path& path : it->second)
		{
			out.push_back("-isystem");
			out.push_back(path.string());
		}
	}
}

static void compiler_to_language_flag(const std::string& compiler, std::vector<std::string>& out)
{
	out.push_back("-x");
	if (compiler.find("++") != std::string::npos)
	{
		out.push_back("c++");
	}
	else
	{
		out.push_back("c");
	}
}

std::vector<std::string> get_sanitized_flags(const compile_command& command, const fs::path& file)
{
	assert(file.is_absolute());
	fs::path compiler_dir = command.get_dir().c_str();

	// copy command out into a vector
	std::vector<std::string> in;
	in.reserve(command.size());
	for(unsigned i = 0; i < command.size(); ++i)
	{
		in.emplace_back(command.get_arg(i).c_str());
	}

	std::vector<std::string> out;

	// skip all the non-dash started flags preceeding compiler (all the cchaces etc)
	auto first_flag_it = std::find_if(in.begin(), in.end(),
		[&](const std::string& f) { return f.size() > 0 && f[0] == '-'; });

	// get common flags for the compiler
	if (first_flag_it != in.begin())
	{
		auto compiler = *(first_flag_it - 1);
		get_common_flags(compiler, out);
		compiler_to_language_flag(compiler, out);
	}
	else
	{
		// assume another compiler
		LOG("Unable to identify compiler, trying g++");
		get_common_flags("g++", out);
	}

	// sanitize
	static const char* STATE_FLGAS_TO_SKIP[] = {
		"-c", "-MP", "--fcolor-diagnostics", "-fcolor-diagnostics", "-fdiagnostics-color=always", "-fdiagnostics-color=auto"
		};

	static const char* PARAM_FLAGS_TO_SKIP[]= {
		"-MD", "-MMD", "-MF", "-MT", "-MQ", "-o", "-Xclang", "-arch"
		};


	bool skip_next = false;
	for(auto it = first_flag_it; it != in.end(); it++)
	{
		if (skip_next)
		{
			skip_next = false;
			continue;
		}

		if (std::find(std::begin(STATE_FLGAS_TO_SKIP), std::end(STATE_FLGAS_TO_SKIP), *it) != std::end(STATE_FLGAS_TO_SKIP))
		{
			continue;
		}

		if (std::find(std::begin(PARAM_FLAGS_TO_SKIP), std::end(PARAM_FLAGS_TO_SKIP), *it) != std::end(PARAM_FLAGS_TO_SKIP))
		{
			skip_next = true;
			continue;
		}

		// remove file name
		if (fs::absolute(fs::path(*it), compiler_dir) == file)
			continue;

		out.push_back(*it);
	}

	return out;
}}

}

