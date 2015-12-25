#pragma once

#include <boost/operators.hpp>

#include <ostream>
#include <vector>

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

// Token that's lmimted to one line
struct line_token
{
	unsigned begin; // index of the fist character of the token
	unsigned end; // index _past_ the last character of the token (end-begin = length)
	token_type type;

	bool operator==(const line_token& o) const { return begin == o.begin  && end == o.end && type == o.type; }
};

std::ostream& operator << (std::ostream& s, const line_token& t);

struct document_position : public boost::partially_ordered<document_position>
{
	unsigned line = 0;
	unsigned column = 0;

	document_position() = default;
	document_position(unsigned l, unsigned c) : line(l), column(c) {}

	bool operator<(const document_position& o) const
	{
		if (line == o.line)
			return column < o.column;
		else
			return line < o.line;
	}

	bool operator == (const document_position& o) const
	{
		return line == o.line && column == o.column;
	}
};

template<typename Writer> void serialize(Writer& writer, const document_position& m)
{
	serialize(writer, m.line);
	serialize(writer, m.column);
}
template<typename Reader> void deserialize(Reader& reader, document_position& m)
{
	deserialize(reader, m.line);
	deserialize(reader, m.column);
}


struct document_range
{
	document_position start;
	document_position end;
};
template<typename Writer> void serialize(Writer& writer, const document_range& m)
{
	serialize(writer, m.start);
	serialize(writer, m.end);
}
template<typename Reader> void deserialize(Reader& reader, document_range& m)
{
	deserialize(reader, m.start);
	deserialize(reader, m.end);
}

// Token that can spawn multiple lines
struct token
{
	token_type type;
	document_range range;
};
template<typename Writer> void serialize(Writer& writer, const token& m)
{
	serialize(writer, m.type);
	serialize(writer, m.range);
}
template<typename Reader> void deserialize(Reader& reader, token& m)
{
	deserialize(reader, m.type);
	deserialize(reader, m.range);
}




}}
