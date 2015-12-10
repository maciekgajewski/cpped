#include "line_data.hh"

#include <algorithm>

namespace cpped { namespace	document {

void line_data::copy_and_insert(const line_data& source, unsigned column, const std::string& text)
{
	assert(column <= length_ && "column out of range");
	assert(tokens_.empty() && "tokens should be empty, this function should be called immediately after construction");
	assert(source.length_ + text.size() == length_ && "invalid line length");

	tokens_ = source.tokens_;
	for(line_token& token : tokens_)
	{
		if (token.end > column)
		{
			token.end += text.length();
			if (token.begin > column)
				token.begin += text.length();
		}
	}
}

void line_data::copy_truncated(const line_data& source, unsigned column, unsigned inserted_length)
{
	assert(column <= length_ && "column out of range");
	assert(tokens_.empty() && "tokens should be empty, this function should be called immediately after construction");
	assert(column + inserted_length == length_);

	tokens_.reserve(source.tokens_.size()); // upper bound

	std::find_if(source.tokens_.begin(), source.tokens_.end(),
		[&](const line_token& token)
		{
			if (token.end < column)
			{
				tokens_.push_back(token);
				return false;
			}
			else
			{
				if (token.begin < column)
				{
					tokens_.push_back(token);
					tokens_.back().end = column + inserted_length;
				}
				return true;
			}
	});
}

void line_data::copy_remainder(const line_data& source, unsigned column, unsigned inserted_length)
{
	assert(tokens_.empty() && "tokens should be empty, this function should be called immediately after construction");

	auto first_to_copy = std::find_if(source.tokens_.begin(), source.tokens_.end(),
		[&](const line_token& token)
		{
			return token.end > column;
		});

	tokens_.reserve(source.tokens_.end() - first_to_copy);
	std::for_each(first_to_copy, source.tokens_.end(),
		[&](const line_token& token)
		{
			tokens_.push_back(token);
			if (tokens_.back().begin >= column)
			{
				tokens_.back().begin += inserted_length - column;
			}
			else
			{
				// text inserted in the middle of token, truncate the token
				tokens_.back().begin = 0;
			}
			tokens_.back().end += inserted_length - column;
		});
}

void line_data::push_back_token(const line_token& t)
{
	assert(t.end <= length_);
	assert(t.begin >= (tokens_.empty() ? 0u : tokens_.back().end));
	tokens_.push_back(t);
}


}}
