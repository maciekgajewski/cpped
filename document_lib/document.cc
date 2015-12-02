#include "document.hh"
#include "clang.hh"

#include <boost/tokenizer.hpp>

#include <fstream>
#include <iostream>

namespace cpped { namespace  document {

std::ostream& operator << (std::ostream& s, token_type tt)
{
	switch(tt)
	{
		case token_type::none : return s << "none";
		case token_type::keyword : return s << "keyword";
		case token_type::literal : return s << "literal";
		case token_type::preprocessor : return s << "preprocessor";
		case token_type::type : return s << "type";
		case token_type::comment : return s << "comment";
		case token_type::max_tokens : return s << "max_tokens";
	}
	return s << "?";
}

std::ostream& operator << (std::ostream& s, const line_token& t)
{
	return s << "token{b="<< t.begin << ", e=" << t.end << ", t=" << t.type << "}";
}


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
	clang::translation_unit tu;
	tu.parse(index, file_name.c_str(), raw_data.data(), raw_data.size());

	clang::source_file file = tu.get_file(file_name);

	// full file tokenizer. doing it line-by-line is not a good approach
	clang::source_location file_begin = tu.get_location_for_offset(file, 0);
	clang::source_location file_end = tu.get_location_for_offset(file, raw_data.size());

	clang::source_range range(file_begin, file_end);
	clang::token_list tokens = tu.tokenize(range);
	tokens.annotate_tokens();

	// clear all tokens in al lines
	for(auto& line : lines)
		line.clear_tokens();

	// sanity check
	if (!lines.empty() && tokens.size() == 0)
		throw std::runtime_error("parsing failed");

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

		line_token lt;
		lt.type = determin_token_type(token);
		lt.begin = range_begin.column - 1;
		document_line* line = &lines.at(line_idx);
		while (line_idx < last_line_idx)
		{
			lt.end = line->get_length();
			line->push_back_token(lt);
			lt.begin = 0;

			line_idx++;
			line = &lines.at(line_idx);
		}
		lt.end = range_end.column-1;
		line->push_back_token(lt);
	}

}

void document::insert(const char* position, char c)
{
	const char* old_data = nullptr;
	if (raw_data.size() + 1 > raw_data.capacity())
	{
		// this will reallocate. Preserve original data ptr
		old_data = raw_data.data();
	}
	raw_data.insert(raw_data.begin() + (position-raw_data.data()), c);

	if (old_data)
	{
		// old_data is invalid now, do not dereference!
		for(document_line& line : lines)
		{
			line.refresh_position(old_data, raw_data.data());
		}
	}
}

void document::shift_lines(document_line* after, unsigned shift)
{
	auto it = lines.begin() + (after-lines.data());
	for(it++; it != lines.end(); it++)
	{
		it->shift(shift);
	}

}

void document::insert_line(document_line* after, document_line&& new_line)
{
	auto it = lines.begin() + (after-lines.data()) + 1;
	lines.insert(it, std::move(new_line));
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
		lines.emplace_back(*this, const_cast<char*>(&*token.begin), token.end - token.begin);
		assert(lines.back().get_data() >= raw_data.data() && lines.back().get_data() <= raw_data.data()+raw_data.size());
	}
}

void document_line::push_back_token(const line_token& t)
{
	assert(t.end <= length);
	assert(t.begin >= (tokens.empty() ? 0u : tokens.back().end));
	tokens.push_back(t);
}

void document_line::insert(unsigned position, char c)
{
	assert(position <= get_length());

	// modify the underlying data buffer
	parent.insert(begin+position, c);

	if (c == '\n')
	{
		document_line new_line(parent, begin+position+1, length - position);

		length = position;
		// move tokens to the next line
		auto first_to_move = std::find_if(tokens.begin(), tokens.end(),
			[&](const line_token& t)
			{
				return t.end > position;
			});

		if (first_to_move != tokens.end())
		{
			unsigned tokens_to_move = tokens.size() - (tokens.begin()-first_to_move);
			if (first_to_move->begin < position)
			{
				tokens_to_move++;
			}
			new_line.tokens.reserve(tokens_to_move);
			if (first_to_move->begin < position)
			{
				new_line.tokens.push_back(line_token{0, first_to_move->end-position, first_to_move->type});
				first_to_move->end = position;
				first_to_move ++;
			}

			std::transform(first_to_move, tokens.end(), std::back_inserter(new_line.tokens),
				[&](const line_token& token)
				{
					return line_token{token.begin-position, token.end-position, token.type};
				});

			tokens.erase(first_to_move, tokens.end());
		}

		parent.shift_lines(this, 1);
		parent.insert_line(this, std::move(new_line)); // last statement, as this may delete this
	}
	else
	{
		// modify tokens
		for(line_token& token : tokens)
		{
			if (token.end > position)
			{
				token.end++;
				if (token.begin > position)
					token.begin++;
			}
		}
		length++;
		parent.shift_lines(this, 1);
	}

}


}}
