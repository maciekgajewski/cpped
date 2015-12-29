#include "completer.hh"

namespace cpped {

completer::completer(project& pr, nct::event_dispatcher& ed, nct::event_window* parent)
	: nct::event_window(ed, parent), project_(pr)
{
}

void completer::activate(const document::document& doc, const document::document_position& cursor_pos)
{
	// start with current cursor position, and move back until it's no longer a valid identifier
	document::document_line& line = doc_->get_line(cursor_pos.line);

	unsigned token_end = cursor_pos.column;
	unsigned token_start = cursor_pos.column;
	while(token_start > 0 && completer::is_valid_identifier(line.get_data() + token_start - 1, token_end-token_start + 1))
	{
		token_start--;
	}

	auto result = project_.get_completion(doc, {cursor_pos.line, token_start});

	// TODO filter, show
}

static bool is_valid_first(char c)
{
	return
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_valid(char c)
{
	// TODO traits, unicode and all that crap
	return
		(c >= 0 && c <= 9) || is_valid_first(c);
}

bool completer::is_valid_identifier(const char* s, unsigned len)
{
	if (len > 0)
	{
		if (!is_valid_first(s[0]))
			return false;
		for(unsigned i = 1; i < len; i++)
		{
			if (!is_valid(s[i]))
				return false;
		}
	}
	return true;
}


}
