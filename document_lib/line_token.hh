#pragma once

#include <boost/operators.hpp>

#include <ostream>

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

	bool operator==(const line_token& o) const { return begin == o.begin  && end == o.end && type == o.type; }
};

std::ostream& operator << (std::ostream& s, const line_token& t);

struct position : public boost::partially_ordered<position>
{
	unsigned line;
	unsigned column;

	position(unsigned l, unsigned c) : line(l), column(c) {}

	bool operator<(const position& o) const
	{
		if (line == o.line)
			return column < o.column;
		else
			return line < o.line;
	}

	bool operator == (const position& o) const
	{
		return line == o.line && column == o.column;
	}
};

struct range
{
	position start;
	position end;
};

}}
