#pragma once

#include "document_lib/document_data.hh"

#include "clang_lib/clang.hh"

#include <boost/filesystem.hpp>

#include <vector>

namespace cpped { namespace backend {

// Builds tokens for a file in the transaltion unit
std::vector<document::token> get_cpp_tokens(const clang::translation_unit& tu,
	const boost::filesystem::path& file_name,
	const std::vector<char>& raw_data);

}}
