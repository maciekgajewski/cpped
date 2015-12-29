#pragma once

#include "clang.hh"

#include <boost/filesystem.hpp>

#include <vector>
#include <string>

namespace cpped { namespace clang {

// turns flags loaded from compilation database into set useful for libclang parse functions
std::vector<std::string> get_sanitized_flags(const compile_command& command, const boost::filesystem::path& file);

void get_common_flags(std::vector<std::string>& out);

}}
