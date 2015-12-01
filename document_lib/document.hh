#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <cassert>

namespace cpped { namespace  document {

//  additional info associated with token

enum class token_type
{
	none,
	keyword,
	literal,
	preprocessor,
	type,
	comment,

	max_tokens
};

std::ostream& operator << (std::ostream& s, token_type tt);

struct line_token
{
	unsigned begin; // index of the fist character of the token
	unsigned end; // index _past_ the last character of the token (end-begin = length)
	token_type type;
};

class document_line
{
public:
	document_line(char* b, unsigned l) : begin(b), length(l) {}

	unsigned get_length() const { return length; }
	const char* get_data() const { return begin; }

	std::string to_string() const { return std::string(begin, begin+length); }

	void clear_tokens() { tokens.clear(); }
	void push_back_token(const line_token& t);

	// Calls 'fun' for each token of the line. If section of the line is not covered by a token, empty token is used
	template<typename FUN>
	void for_each_token(FUN fun) const;

	// inserting text
	void insert(unsigned position, char c);

private:

	char* begin;
	unsigned length;
	std::vector<line_token> tokens;
};

class document
{
public:

	void load_from_raw_data(const std::string& data, const std::string& fake_path);
	void load_from_raw_data(std::vector<char> data);
	void load_from_file(const std::string& path);

	unsigned get_line_count() const { return lines.size(); }

	unsigned line_length(unsigned index)
	{
		if (index == lines.size())
			return 0; // fake last line
		else
			return get_line(index).get_length();
	}

	document_line& get_line(unsigned index) { return lines.at(index); }
	const document_line& get_line(unsigned index) const { return lines.at(index); }

	// iterates over no more than 'count' lines in range, starting from first_line
	template<typename FUN>
	void for_lines(unsigned first_line, unsigned max_count, FUN f)
	{
		if (first_line < lines.size())
		{
			unsigned count = std::min<unsigned>(lines.size()-first_line, max_count);
			auto it = lines.begin() + first_line;
			for(unsigned i = 0; i < count; ++i, ++it)
			{
				f(*it);
			}
		}
	}

	void parse_language();

	const std::vector<char>& get_raw_data() const { return raw_data; }

private:

	void parse_raw_buffer();

	std::vector<char> raw_data;
	std::vector<document_line> lines;
	std::string file_name;

};

template<typename FUN>
void document_line::for_each_token(FUN fun) const
{
	unsigned current = 0;
	auto it = tokens.begin();
	line_token none_token { 0, 0, token_type::none };

	while(current < length)
	{
		if (it == tokens.end())
		{
			none_token.begin = current;
			none_token.end = length;
			fun(none_token);
			current = length;
		}
		else
		{
			const line_token& token = *it;
			it++;
			assert(token.begin >= current);
			assert(token.end <= length);
			if (token.begin > current)
			{
				none_token.begin = current;
				none_token.end = token.begin;
				fun(none_token);
			}
			fun(token);
			current = token.end;
		}
	}
}


}}
