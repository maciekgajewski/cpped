#include "cpp_parser.hh"

#include "document.hh"

namespace cpped { namespace document {

cpp_parser::cpp_parser()
:	index(0, 0)
{
}

static token_type determine_token_type(const clang::token& token)
{
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



void cpp_parser::parse(document& doc)
{
	const auto& raw_data = doc.get_raw_data();
	const std::string& file_name = doc.get_file_name();

	if (tu.is_null())
	{
		// first parsing
		tu.parse(index, file_name.c_str(), raw_data.data(), raw_data.size());
	}
	else
	{
		tu.reparse(file_name.c_str(), raw_data.data(), raw_data.size());
	}

	clang::source_file file = tu.get_file(file_name);

	// full file tokenizer. doing it line-by-line is not a good approach
	clang::source_location file_begin = tu.get_location_for_offset(file, 0);
	clang::source_location file_end = tu.get_location_for_offset(file, raw_data.size());

	clang::source_range range(file_begin, file_end);
	clang::token_list tokens = tu.tokenize(range);
	tokens.annotate_tokens();

	// clear all tokens in al lines
	doc.for_lines(0, doc.get_line_count(), [&](auto& line)
	{
		line.clear_tokens();
	});

	// sanity check
	if (doc.get_line_count() > 0 && tokens.size() == 0)
		throw std::runtime_error("parsing failed");

	for(const clang::token& token : tokens)
	{
		clang::source_range range = tokens.get_token_extent(token);
		clang::source_location::info range_begin = range.get_start().get_location_info();
		clang::source_location::info range_end = range.get_end().get_location_info();

		unsigned line_idx = range_begin.line-1;
		unsigned last_line_idx = range_end.line-1;
		assert(line_idx < doc.get_line_count());
		assert(last_line_idx < doc.get_line_count());
		assert(line_idx <= last_line_idx);

		line_token lt;
		lt.type = determine_token_type(token);
		lt.begin = range_begin.column - 1;
		document_line* line = &doc.get_line(line_idx);
		while (line_idx < last_line_idx)
		{
			lt.end = line->get_length();
			line->push_back_token(lt);
			lt.begin = 0;

			line_idx++;
			line = &doc.get_line(line_idx);
		}
		lt.end = range_end.column-1;
		line->push_back_token(lt);
	}
}


}}

