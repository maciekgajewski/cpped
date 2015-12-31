#include "cpp_tokens.hh"

#include "log.hh"

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
		if (cursor_kind == CXCursor_NamespaceRef || cursor_kind == CXCursor_TypeRef || cursor_kind == CXCursor_TemplateRef)
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

	LOG("got " << tokens.size() << " tokens for " << file_name);

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

document::token_data get_cpp_tokens_with_diagnostics(const clang::translation_unit& tu,
	const boost::filesystem::path& file_name,
	const std::vector<char>& raw_data)
{
	document::token_data data;

	// start by just getting tokens
	data.tokens = get_cpp_tokens(tu, file_name, raw_data);

	// count the diagnostics
	clang::diagnostic_set diags = tu.get_diagnostics();
	LOG("Reading diagnostics for file: " << file_name << ", diags: " << diags.size());
	for(auto i = 0u; i < diags.size(); i++)
	{
		clang::diagnostic diag = diags[i];
		std::string message = diag.format(CXDiagnostic_DisplaySourceLocation|CXDiagnostic_DisplaySourceRanges).c_str();
		LOG(" * " << message);
		for(auto r = 0u; r < diag.get_num_ranges(); r++)
		{
			clang::source_range rng = diag.get_range(r);
			auto start = rng.get_start().get_location_info();
			auto end = rng.get_end().get_location_info();
			LOG("    " << start.line << ":" << start.column << " - " << end.line << ":" << end.column);
		}


		switch(diag.get_severity())
		{
			case CXDiagnostic_Ignored:
			case CXDiagnostic_Note:
				break;
			case CXDiagnostic_Warning:
				data.diagnostics.push_back({message, document::problem_severity::warning});
				break;
			case CXDiagnostic_Error:
			case CXDiagnostic_Fatal:
				data.diagnostics.push_back({message, document::problem_severity::error});
				break;
		}
	}

	return data;
}

}}
