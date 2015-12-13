#include "line_edit.hh"

#include "event_dispatcher.hh"

namespace nct {

line_edit::line_edit(event_dispatcher& ed, event_window* parent)
	: event_window(ed, parent)
{
}

void line_edit::set_text(const std::__cxx11::string& t)
{
	text_ = t;
	first_column_ = 0;
}

unsigned line_edit::on_sequence(const std::string& s)
{
	assert(cursor_pos_ <= text_.size());

	// consume everything up to the first \n
	auto endl_pos = std::find(s.begin(), s.end(), '\n');

	if (endl_pos != s.begin())
	{
		text_.insert(text_.begin() + cursor_pos_, s.begin(), endl_pos);
		cursor_pos_ += endl_pos - s.begin();
		text_changed(text_);
		update();
	}
	if (endl_pos != s.end())
	{
		enter_pressed();
	}

	return endl_pos - s.begin();
}

bool line_edit::on_special_key(int key_code, const char* key_name)
{
	switch(key_code)
	{
		case KEY_LEFT:
			cursor_left(); return true;
		case KEY_RIGHT:
			cursor_right(); return true;
		case KEY_BACKSPACE:
			backspace(); return true;
		case KEY_DC:
			del(); return true;
	}

	return false;
}

void line_edit::on_deactivated()
{
	hints_widget_.reset();
}

void line_edit::on_activated()
{
	show_hints();
}

void line_edit::on_shown()
{
	update();
}

void line_edit::cursor_left()
{
	if (cursor_pos_ > 0)
	{
		cursor_pos_--;
		if (cursor_pos_ < first_column_)
		{
			first_column_ = cursor_pos_;
		}
		update();
	}
}

void line_edit::cursor_right()
{
	if (cursor_pos_ < text_.size())
	{
		cursor_pos_++;
		if (cursor_pos_ + first_column_ >= get_size().w)
		{
			first_column_++;
		}
		update();
	}
}

void line_edit::backspace()
{
	if (cursor_pos_ > 0)
	{
		cursor_pos_--;
		text_.erase(text_.begin() + cursor_pos_);
		text_changed(text_);
		update();
	}
}

void line_edit::del()
{
	if (cursor_pos_ < text_.size())
	{
		text_.erase(text_.begin() + cursor_pos_);
		text_changed(text_);
		update();
	}
}

void line_edit::update()
{
	if (!is_visible()) return;
	ncurses_window& window = get_ncurses_window();

	int text_attr = COLOR_PAIR(get_palette().get_pair_for_colors(COLOR_CYAN, COLOR_BLACK));
	int help_text_attr = COLOR_PAIR(get_palette().get_pair_for_colors(COLOR_CYAN, COLOR_BLUE));
	window.set_attr_on(text_attr);

	// background
	window.move_cursor(0, 0);
	for(int x = 0; x < get_size().w; x++)
		window.put_char(' ');

	// text
	window.move_cursor(0, 0);
	if (text_.empty())
	{
		window.set_attr_on(help_text_attr);
		window.print(help_text_);
	}
	else if (first_column_ < text_.size())
	{
		window.print(text_.c_str() + first_column_, get_size().w);
	}

	// cursor
	if (cursor_pos_ >= first_column_)
	{
		show_cursor({0, int(cursor_pos_-first_column_)});
	}
	else
	{
		hide_cursor();
	}

	refresh_window();
}

void line_edit::hints_changed()
{
	if (hints_.empty())
	{
		hints_widget_.reset();
	}
	else
	{
		if (!hints_widget_.is_initialized())
		{
			hints_widget_.emplace(get_event_dispatcher(), this);
		}
		if (is_active() && is_visible())
		{
			show_hints();
		}
	}
}

void line_edit::show_hints()
{
	if (hints_.empty()) return;

	// create hitns
	std::vector<list_widget::list_item> items;
	items.reserve(hints_.size());
	std::transform(hints_.begin(), hints_.end(), std::back_inserter(items),
			[&](const completion_hint& hint)
			{
				return list_widget::list_item{hint.text, hint.help_text};
			});

	hints_widget_->set_items(items);

	size content_size = hints_widget_->get_content_size();

	position global_pos = to_global(position{0, 1});

	size sz;
	int max_h = std::min(LINES - global_pos.y, 40);
	int max_w = std::min(COLS - global_pos.x, 80);

	sz.h = std::min(max_h, content_size.h);
	sz.w = std::min(max_w, content_size.w);
	hints_widget_->move(position{0, 1}, sz);

	hints_widget_->show();
}

}
