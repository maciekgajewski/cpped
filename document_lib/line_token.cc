#include "line_token.hh"

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


}}
