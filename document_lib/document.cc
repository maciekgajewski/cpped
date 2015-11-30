#include "document.hh"
#include "clang.hh"

#include <boost/tokenizer.hpp>

#include <fstream>
#include <iostream>

namespace cpped { namespace  document {

void document::load_from_raw_data(const std::string& data, const std::string& fake_path)
{
	file_name = fake_path;
	raw_data.assign(data.begin(), data.end());
	parse_raw_buffer();
}

void document::load_from_file(const std::string& path)
{
	file_name = path;
	std::fstream f(path, std::ios_base::in);
	if (f.fail())
	{
		throw std::runtime_error("Error opening file");
	}

	raw_data.clear();

	const unsigned bufsize = 4096;
	char buf[bufsize];
	auto c = f.readsome(buf, bufsize);
	while( c > 0)
	{
		raw_data.insert(raw_data.end(), buf, buf+c);
		c = f.readsome(buf, bufsize);
	}

	parse_raw_buffer();
}

static token_type determin_token_type(const clang::token& token)
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

void document::parse_language()
{
	clang::index index(0, 0);
	clang::translation_unit tu(index, file_name.c_str(), raw_data.data(), raw_data.size());

	clang::source_file file = tu.get_file(file_name);

	// full file tokenizer. doing it line-by-line is not a good approach
	clang::source_location file_begin = tu.get_location(file, 1, 1);
	clang::source_location file_end = tu.get_location(file, lines.size() + 1, lines.back().get_length());
	clang::source_range range(file_begin, file_end);
	clang::token_list tokens = tu.tokenize(range);
	tokens.annotate_tokens();

	// clear all tokens in al lines
	for(auto& line : lines)
		line.clear_tokens();

	for(const clang::token& token : tokens)
	{
		clang::source_range range = tokens.get_token_extent(token);
		clang::source_location::info range_begin = range.get_start().get_location_info();
		clang::source_location::info range_end = range.get_end().get_location_info();

		unsigned line_idx = range_begin.line-1;
		unsigned last_line_idx = range_end.line-1;
		assert(line_idx < lines.size());
		assert(last_line_idx < lines.size());
		assert(line_idx <= last_line_idx);

		do
		{
			document_line& line = lines.at(line_idx);
			line_token lt;
			lt.begin = range_begin.column - 1;
			lt.type = determin_token_type(token);
			if (line_idx == last_line_idx)
			{
				lt.end = range_end.column-1;
				line.push_back_token(lt);
			}
			else
			{
				lt.end = line.get_length();
				line_idx++;
				lt.begin = 0;
			}
		} while (line_idx < last_line_idx);
	}

#if 0 // debug dump
	std::cout << "entire file: tokens=" << tokens.size() << std::endl;
	unsigned line_number = 0;
	for(const clang::token& token : tokens)
	{
		clang::source_location location = tokens.get_token_location(token);
		clang::source_location::info info = location.get_location_info();
		if (info.line != line_number)
		{
			line_number = info.line;
			std::cout << std::endl;
			std::cout << "line: " << line_number << " '" << lines[line_number-1].to_string() << std::endl;
		}
		if (token.has_associated_cursor())
		{
			clang::string type = token.get_associated_cursor().get_kind_as_string();
			std::cout << token.get_kind_name() << " (" << type << "), ";
		}
		else
		{
			std::cout << token.get_kind_name() << ", ";
		}
	}
	std::cout << std::endl;
#endif
}

struct range
{
	std::vector<char>::const_iterator begin;
	std::vector<char>::const_iterator end;

	void assign(std::vector<char>::const_iterator b, std::vector<char>::const_iterator e)
	{
		begin = b;
		end = e;
	}
};

void document::parse_raw_buffer()
{
	lines.clear();

	// tokenize by lines
	boost::char_separator<char> endline_sep("\n", "", boost::keep_empty_tokens);
	boost::tokenizer<decltype(endline_sep), std::vector<char>::const_iterator, range> tokens(raw_data, endline_sep);

	for(const range& token : tokens)
	{
		lines.emplace_back(const_cast<char*>(&*token.begin), token.end - token.begin);
		assert(lines.back().get_data() >= raw_data.data() && lines.back().get_data() <= raw_data.data()+raw_data.size());
	}
}

void document_line::push_back_token(const line_token& t)
{
	assert(t.end < length);
	assert(t.begin > (tokens.empty() ? 0u : tokens.back().end));
	tokens.push_back(t);
}


}}
