#pragma once

#include "document_lib/document_data.hh"

#include "clang_lib/clang.hh"

#include <boost/filesystem.hpp>

#include <vector>

namespace cpped { namespace backend {

struct token_data
{
	std::vector<document::token> tokens;
	unsigned errors = 0;
	unsigned warnings = 0;
};
template<typename Writer> void serialize(Writer& writer, const token_data& m)
{
	serialize(writer, m.tokens);
	serialize(writer, m.errors);
	serialize(writer, m.warnings);
}
template<typename Reader> void deserialize(Reader& reader, token_data& m)
{
	deserialize(reader, m.tokens);
	deserialize(reader, m.errors);
	deserialize(reader, m.warnings);
}


// Builds tokens for a file in the transaltion unit
std::vector<document::token> get_cpp_tokens(const clang::translation_unit& tu,
	const boost::filesystem::path& file_name,
	const std::vector<char>& raw_data);

token_data get_cpp_tokens_with_diagnostics(const clang::translation_unit& tu,
	const boost::filesystem::path& file_name,
	const std::vector<char>& raw_data);

}}
