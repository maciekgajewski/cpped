#pragma once

#include "iparser.hh"

#include "details/document_data.hh"

#include <vector>
#include <string>
#include <ostream>
#include <cassert>
#include <memory>
#include <chrono>
#include <list>

namespace cpped { namespace  document {

class document;


class document_line
{
public:

	document_line(document_line&&) = default;
	document_line(document& doc, char* b, unsigned l) : parent(doc), begin(b), length(l) {}

	unsigned get_length() const { return length; }
	const char* get_data() const { return begin; }

	std::string to_string() const { return std::string(begin, begin+length); }

	// Calls 'fun' for each token of the line. If section of the line is not covered by a token, empty token is used
	template<typename FUN>
	void for_each_token(FUN fun) const;

	const std::vector<line_token>& get_tokens() const { return tokens;} // for testing

private:

	details::line_data& line_;
};

struct position
{
	unsigned line;
	unsigned column;
};

class document
{
public:

	document();
	~document();

	void load_from_raw_data(const std::string& data, const std::string& fake_path, std::unique_ptr<iparser>&& parser = nullptr);
	void load_from_file(const std::string& path, std::unique_ptr<iparser>&& parser = nullptr);

	unsigned get_line_count() const { return lines_.size(); }

	unsigned line_length(unsigned index)
	{
		if (index == lines_.size())
			return 0; // fake last line
		else
			return get_line(index).get_length();
	}

	document_line& get_line(unsigned index) { return lines_.at(index); }
	const document_line& get_line(unsigned index) const { return lines_.at(index); }

	// insert string at location. Returns the position of the end of inserted text
	position insert(position pos, const std::string& text);

	// iterates over no more than 'count' lines in range, starting from first_line
	template<typename FUN>
	void for_lines(unsigned first_line, unsigned max_count, FUN f)
	{
		if (first_line < lines_.size())
		{
			unsigned count = std::min<unsigned>(lines_.size()-first_line, max_count);
			auto it = lines_.begin() + first_line;
			for(unsigned i = 0; i < count; ++i, ++it)
			{
				f(*it);
			}
		}
	}

	void parse_language();

	const std::vector<char>& get_raw_data() const { return raw_data_; }
	const std::string& get_file_name() const { return file_name; }

	std::string to_string() const; // mostly for testing

	void insert(const char* position, char c);

	void shift_lines(document_line* after, unsigned shift);
	void insert_line(document_line* after, document_line&& new_line);

	std::chrono::high_resolution_clock::duration get_last_parse_time() const { return last_parse_time; }

private:

	std::vector<details::document_data> data_;
	std::vector<details::document_data>::const_iterator current_data_;

	std::string file_name;

	std::unique_ptr<iparser> parser;
	std::chrono::high_resolution_clock::duration last_parse_time;
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
