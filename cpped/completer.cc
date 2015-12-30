#include "completer.hh"

namespace cpped {

completer::completer(project& pr, nct::event_dispatcher& ed, nct::event_window* parent)
	: nct::event_window(ed, parent), project_(pr)
{
	move({0, 0}, {1, 1}); // TODO ugly hack, fix
}

void completer::activate(
	const document::document& doc,
	const document::document_position& cursor_pos,
	const nct::position& screen_pos)
{
	// start with current cursor position, and move back until it's no longer a valid identifier
	const document::document_line& line = doc.get_line(cursor_pos.line);

	unsigned token_end = cursor_pos.column;
	unsigned token_start = cursor_pos.column;
	while(token_start > 0 && completer::is_valid_identifier(line.get_data() + token_start - 1, token_end-token_start + 1))
	{
		token_start--;
	}

	auto result = project_.get_completion(doc, {cursor_pos.line, token_start});

	if (result.empty())
	{
		return;
	}

	filter_.assign(line.get_data() + token_start, line.get_data() + token_end);
	set_position(screen_pos);

	std::vector<nct::list_widget::list_item> items;
	items.reserve(result.size());

	for(backend::messages::completion_record& record : result)
	{
		items.push_back({std::move(record.text), std::move(record.hint)});
	}
	list_.emplace(get_event_dispatcher(), this);
	list_->set_items(items);
	list_->set_size({10, list_->get_content_size().w});
	list_->set_position(screen_pos);
	list_->show();
	list_->set_active();
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
