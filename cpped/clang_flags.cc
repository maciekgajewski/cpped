#include "clang_flags.hh"

#include <regex>

// What I'm doing here is (mostly) based on what YCMD is doing

namespace cpped {

namespace fs = boost::filesystem;

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

std::vector<std::string> sanitize_clang_flags(
	const std::vector<std::string>& in,
	const fs::path& file, const fs::path& compiler_dir)
{
	assert(file.is_absolute());

	std::vector<std::string> out;

	// skip all the non-dash started flags preceeding compiler (all the cchaces etc)
	auto first_flag_it = std::find_if(in.begin(), in.end(),
		[&](const std::string& f) { return f.size() > 0 && f[0] == '-'; });

	// convert compiler to -x flag
	if (first_flag_it != in.begin())
	{
		compiler_to_language_flag(*(first_flag_it - 1), out);
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

	// add comon required flags
	out.push_back("-fdiagnostics-color=never");

	return out;
}

}

