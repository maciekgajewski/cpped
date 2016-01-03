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

enum class problem_severity
{
	none,

	note,
	warning,
	error
};

std::ostream& operator << (std::ostream& s, token_type tt);

// Token that's lmimted to one line
struct line_token
{
	unsigned begin; // index of the fist character of the token
	unsigned end; // index _past_ the last character of the token (end-begin = length)
	token_type type;

	// diagnositcs
	std::uint32_t diagnositc_index;

	bool operator==(const line_token& o) const { return begin == o.begin  && end == o.end && type == o.type; }
};

std::ostream& operator << (std::ostream& s, const line_token& t);

struct document_position : public boost::totally_ordered<document_position>
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

struct diagnostic_message
{
	std::string message;
	problem_severity severity;
	// TODO range and fixi-it goes here
};
template<typename Writer> void serialize(Writer& writer, const diagnostic_message& m)
{
	serialize(writer, m.message);
	serialize(writer, m.severity);
}
template<typename Reader> void deserialize(Reader& reader, diagnostic_message& m)
{
	deserialize(reader, m.message);
	deserialize(reader, m.severity);
}

// Token that can spawn multiple lines
struct token
{
	token_type type;
	document_range range;
	std::uint32_t diagnostic_index;
};
template<typename Writer> void serialize(Writer& writer, const token& m)
{
	serialize(writer, m.type);
	serialize(writer, m.range);
	serialize(writer, m.diagnostic_index);
}
template<typename Reader> void deserialize(Reader& reader, token& m)
{
	deserialize(reader, m.type);
	deserialize(reader, m.range);
	deserialize(reader, m.diagnostic_index);
}

struct token_data
{
	std::vector<token> tokens;
	std::vector<diagnostic_message> diagnostics;
};
template<typename Writer> void serialize(Writer& writer, const token_data& m)
{
	serialize(writer, m.tokens);
	serialize(writer, m.diagnostics);
}
template<typename Reader> void deserialize(Reader& reader, token_data& m)
{
	deserialize(reader, m.tokens);
	deserialize(reader, m.diagnostics);
}



}}
