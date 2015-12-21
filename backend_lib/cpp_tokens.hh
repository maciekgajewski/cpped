#pragma once

#include "document_lib/document_data.hh"

#include "clang_lib/clang.hh"

#include <boost/filesystem.hpp>

#include <vector>

namespace cpped { namespace backend {

// Reparses translation unitm resuts list of tokens
// TODO split reparsing from getting tokens
std::vector<document::token> get_cpp_tokens(
	clang::translation_unit& tu,
	const boost::filesystem::path& file_name,
	const std::vector<char>& raw_data);

}}
