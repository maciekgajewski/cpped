#include "completer_widget.hh"

namespace cpped {


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


completer_widget::completer_widget(project& pr, nct::window_manager& ed, nct::event_window* parent)
	: nct::event_window(ed, parent), project_(pr)
{
}

void completer_widget::activate(
	const document::document& doc,
	const document::document_position& cursor_pos,
	const nct::position& screen_pos)
{
	// start with current cursor position, and move back until it's no longer a valid identifier
	const document::document_line& line = doc.get_line(cursor_pos.line);

	unsigned token_end = cursor_pos.column;
	unsigned token_start = cursor_pos.column;
	while(token_start > 0 && completer_widget::is_valid_identifier(line.get_data() + token_start - 1, token_end-token_start + 1))
	{
		token_start--;
	}

	auto result = project_.get_completion(doc.get_file_name(), {cursor_pos.line, token_start});

	if (result.empty())
	{
		return;
	}

	start_position_ = cursor_pos;
	start_position_.column = token_start;
	filter_.assign(line.get_data() + token_start, line.get_data() + token_end);
	set_position(screen_pos + nct::position{1,-int(filter_.size())});

	std::vector<nct::list_widget::list_item> items;
	items.reserve(result.size());

	for(backend::messages::completion_record& record : result)
	{
		items.push_back({std::move(record.text) /* no help text*/});
	}
	list_.emplace(get_window_manager(), this);
	list_->set_items(items);
	list_->set_filter(filter_);

	// if there is only one condidate, select it immediately
	if (list_->get_filtered_count() == 1)
	{
		complete();
	}
	else
	{
		auto cs = list_->get_content_size();

		list_->set_size({std::min(10, cs.h), std::max(40, cs.w)});
		list_->show();
		list_->set_active();
	}
}

unsigned completer_widget::on_sequence(const std::string& s)
{
	assert(list_);

	filter_.reserve(filter_.size() + s.size());
	for(unsigned i = 0; i < s.size(); i++)
	{
		char c = s[i];
		if (c == '\n')
		{
			// if there is something to select from - use it
			if (list_->get_filtered_count() > 0)
			{
				complete();
				return i+1;
				// TODO there is a bug here, filter may be modified, but this is not reflected in the editor
			}
			// oterwise - cancel and pass trough
			else
			{
				cancel();
				return 0;
			}
		}
		if ((filter_.empty() && !is_valid_first(c)) || !is_valid(c))
		{
			// filter no longer represents a valid identifier, deactivate
			cancel();
			return 0;
		}
		filter_.push_back(c);
	}

	list_->set_filter(filter_);
	return 0;
}

bool completer_widget::on_special_key(int key_code, const char* /*key_name*/)
{
	if (key_code == KEY_BACKSPACE && !filter_.empty())
	{
		// apply the backspace to filter
		assert(list_);
		filter_.erase(filter_.size()-1);
		list_->set_filter(filter_);
		return false; // let edtior apply the change
	}
	if(key_code == 27) // ESC
	{
		cancel();
		return true;
	}

	// all the others - cancl completion and pass the key to the editor
	cancel();
	return false;
}

void completer_widget::cancel()
{
	list_.reset();
	completion_cancelled_signal();
}

void completer_widget::complete()
{
	assert(list_);
	auto* item = list_->get_current_item();
	assert(item);
	std::string text = item->text;
	list_.reset();

	completion_signal(start_position_, filter_.size(), text);
}

bool completer_widget::is_valid_identifier(const char* s, unsigned len)
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
