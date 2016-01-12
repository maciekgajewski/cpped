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
		if (token.end >= column)
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

void line_data::copy_and_remove(const line_data& source, unsigned removed_begin, unsigned removed_end)
{
	assert(tokens_.empty());
	assert(removed_end > removed_begin);
	tokens_.reserve(source.tokens_.size()); // upper bound

	for(const line_token& token : source.tokens_)
	{
		if (token.end <= removed_begin)
		{
			// no changes
			tokens_.push_back(token);
		}
		else
		{
			// possibilities:
			// {aabbcc.., a{abbcc.., aab}bcc

			if (token.begin < removed_begin)
			{
				// a{a...
				// end of the token clipped
				tokens_.push_back(token);
				tokens_.back().end = removed_begin;
			}
			else
			{
				// possibilites: aa}bbcc or a}abbcc
				if (token.end <= removed_end)
				{
					// token fully removed: ..aabb}cc
				}
				else
				{
					tokens_.push_back(token);
					tokens_.back().end -= (removed_end-removed_begin);
					if (token.begin < removed_end)
					{
						// front of the token clipped
						tokens_.back().begin = removed_begin;
					}
					else
					{
						// token after the removed are, just shifted
						tokens_.back().begin -= (removed_end-removed_begin);
					}
				}
			}
		}
	}
}

void line_data::copy_from_two_clipped(const line_data& first, unsigned last_column, const line_data& second, unsigned first_column)
{
	assert(tokens_.empty());
	assert(length_ == last_column + second.get_length() - first_column);

	tokens_.reserve(first.tokens_.size() + second.tokens_.size()); // upper bound
	// copy tokens from the first line
	std::find_if(first.tokens_.begin(), first.tokens_.end(),
			[&](const line_token& token)
			{
				if (token.begin < last_column)
				{
					tokens_.push_back(token);
					if (token.end > last_column)
					{
						tokens_.back().end = last_column;
					}
					return false;
				}
				return true; // found the end, stop iterating now
			});

	// copy tokens from the last line
	auto first_to_copy = std::find_if(second.tokens_.begin(), second.tokens_.end(),
			[&](const line_token& token) { return token.end > first_column; });

	std::transform(first_to_copy, second.tokens_.end(), std::back_inserter(tokens_),
			[&](const line_token& token)
			{
				line_token copy = token;
				int shift = int(last_column) - int(first_column);
				copy.end += shift;
				if (copy.begin < first_column)
					copy.begin = last_column;
				else
					copy.begin += shift;

				return copy;
			});
}

void line_data::push_back_token(const line_token& t)
{
	assert(t.end <= length_);
	assert(t.begin >= (tokens_.empty() ? 0u : tokens_.back().end));
	tokens_.push_back(t);
}

const line_token* line_data::get_token_at(unsigned col) const
{
	assert(col <= get_length());

	// find first token ending past the point
	auto it = std::find_if(tokens_.begin(), tokens_.end(),
		[&](const line_token& token) { return token.end > col; });

	if (it == tokens_.end() || it->begin > col)
	{
		return nullptr;
	}
	else
	{
		return &*it;
	}

}

const line_token* line_data::get_token_before(unsigned col) const
{
	assert(col <= get_length());

	if (tokens_.empty())
		return nullptr;

	auto it = std::find_if(tokens_.begin(), tokens_.end(),
		[&](const line_token& token) { return token.end > col; });

	if (it == tokens_.begin())
	{
		return nullptr;
	}
	else
	{
		--it;
		return &*it;
	}
}


}}
