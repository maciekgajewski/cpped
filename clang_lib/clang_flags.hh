#pragma once

#include <boost/filesystem.hpp>

#include <vector>
#include <string>

namespace cpped { namespace clang {

// turns flags loaded from compilation database into set useful for libclang parse functions
std::vector<std::string> sanitize_clang_flags(
	const std::vector<std::string>& original,
	const boost::filesystem::path& file,
	const boost::filesystem::path& compiler_dir
	);

}}
