#pragma once

#include "line_token.hh"

#include <vector>
#include <cassert>

namespace cpped { namespace	document {

using data_type = std::vector<char>;

class line_data
{
public:

	// create token-less line
	line_data(data_type::const_iterator start, unsigned length) : start_(start), length_(length) {}

	// different scenarios of initializing tokens
	void copy_tokens(const line_data& source) { tokens_ = source.tokens_; }

	// copy tokens as if there was a text inserted at column
	void copy_and_insert(const line_data& source, unsigned column, const std::string& text);

	// copy tokens as if text after column was replaced
	void copy_truncated(const line_data& source, unsigned column, unsigned inserted_length);

	// copy remainder of the line after column, with thext inserted at front
	void copy_remainder(const line_data& source, unsigned column, unsigned inserted_length);

	void copy_and_remove(const line_data& source, unsigned removed_begin, unsigned removed_end);

	void copy_from_two_clipped(const line_data& first, unsigned last_column, const line_data& second, unsigned first_column);

	// uset by language parser
	void clear_tokens() { tokens_.clear(); }
	void push_back_token(const line_token& t);

	data_type::const_iterator get_start() const { return start_; }
	unsigned get_length() const { return length_; }

	std::string to_string() const { return std::string(start_, start_+length_); }
	const std::vector<line_token>& get_tokens() const { return tokens_; }

	// Calls 'fun' for each token of the line. If section of the line is not covered by a token, empty token is used
	template<typename FUN>
	void for_each_token(FUN fun) const;

private:
	data_type::const_iterator start_;
	unsigned length_;

	std::vector<line_token> tokens_;
};


template<typename FUN>
void line_data::for_each_token(FUN fun) const
{
	unsigned current = 0;
	auto it = tokens_.begin();
	line_token none_token { 0, 0, token_type::none };

	while(current < length_)
	{
		if (it == tokens_.end())
		{
			none_token.begin = current;
			none_token.end = length_;
			fun(none_token);
			current = length_;
		}
		else
		{
			const line_token& token = *it;
			it++;
			assert(token.begin >= current);
			assert(token.end <= length_);
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

