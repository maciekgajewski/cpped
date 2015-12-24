#include "cpp_tokens.hh"

#include <algorithm>

namespace cpped { namespace backend {

static document::token_type determine_token_type(const clang::token& token)
{
	using document::token_type;
	assert(token.has_associated_cursor());

	CXTokenKind token_kind = token.get_kind();
	CXCursorKind cursor_kind = token.get_associated_cursor().get_kind();

	if (token_kind == CXToken_Keyword)
	{
		return token_type::keyword;
	}
	else if (token_kind == CXToken_Comment)
	{
		return token_type::comment;
	}
	else if (cursor_kind >= CXCursor_FirstPreprocessing && cursor_kind <= CXCursor_LastPreprocessing)
	{
		return token_type::preprocessor;
	}
	else if (token_kind == CXToken_Literal)
	{
		// TODO what kind of literal?
		return token_type::literal;
	}
	else if (token_kind == CXToken_Identifier)
	{
		if (cursor_kind == CXCursor_NamespaceRef || cursor_kind == CXCursor_TypeRef)
			return token_type::type;
	}

	return token_type::none;
}


std::vector<document::token> get_cpp_tokens(
	const clang::translation_unit& tu,
	const boost::filesystem::path& file_name,
	const std::vector<char>& raw_data)
{
	std::vector<document::token> document_tokens;

	clang::source_file file = tu.get_file(file_name.c_str());

	// full file tokenizer. doing it line-by-line is not a good approach
	clang::source_location file_begin = tu.get_location_for_offset(file, 0);
	clang::source_location file_end = tu.get_location_for_offset(file, raw_data.size());

	clang::source_range range(file_begin, file_end);
	clang::token_list tokens = tu.tokenize(range);
	tokens.annotate_tokens();

	document_tokens.reserve(tokens.size());

	std::transform(tokens.begin(), tokens.end(), std::back_inserter(document_tokens),
		[&](const clang::token& token)
		{
			clang::source_range range = tokens.get_token_extent(token);
			clang::source_location::info range_begin = range.get_start().get_location_info();
			clang::source_location::info range_end = range.get_end().get_location_info();

			document::token document_token{
				determine_token_type(token),
				document::document_range{
					document::document_position{range_begin.line - 1, range_begin.column - 1},
					document::document_position{range_end.line - 1, range_end.column - 1}
					}
				};

			return document_token;
		});

	return document_tokens;
}

}}
