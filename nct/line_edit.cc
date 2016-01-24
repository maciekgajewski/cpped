#include "line_edit.hh"

#include "window_manager.hh"
#include "style.hh"

namespace nct {

line_edit::line_edit(window_manager& ed, event_window* parent)
	: event_window(ed, parent)
{
	// default styles
	style_ = nct::style{COLOR_CYAN, COLOR_BLACK};
	help_text_color_ = COLOR_BLUE;
}

void line_edit::set_text(const std::string& t)
{
	text_ = t;
	first_column_ = 0;
	cursor_pos_ = 0;
	request_redraw();
}

void line_edit::move_cursor_to_end()
{
	end();
}

void line_edit::on_text_changed()
{
	if(hints_widget_)
	{
		hints_widget_->set_filter(text_);
		update_hints_size();
	}

	text_changed_signal(text_);
	request_redraw();
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
		on_text_changed();
	}
	if (endl_pos != s.end())
	{
		enter();
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
		case KEY_HOME:
			home(); return true;
		case KEY_END:
			end(); return true;

		// forwarded to line
		case KEY_UP:
			if (hints_widget_)
			{
				hints_widget_->select_previous();
				return true;
			}
			break;
		case KEY_DOWN:
			if (hints_widget_)
			{
				hints_widget_->select_next();
				return true;
			}
			break;
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
	request_redraw();
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
		request_redraw();
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
		request_redraw();
	}
}

void line_edit::backspace()
{
	if (cursor_pos_ > 0)
	{
		cursor_pos_--;
		text_.erase(text_.begin() + cursor_pos_);
		on_text_changed();
	}
}

void line_edit::del()
{
	if (cursor_pos_ < text_.size())
	{
		text_.erase(text_.begin() + cursor_pos_);
		on_text_changed();
	}
}

void line_edit::enter()
{
	enter_pressed_signal();
	if (hints_widget_)
	{
		list_widget::list_item* item = hints_widget_->get_current_item();
		if (item)
		{
			completion_hint hint{item->text, item->help_text};
			hint_selected_signal(hint);
		}
	}
}

void line_edit::end()
{
	unsigned cp = text_.length();
	if (cp != cursor_pos_)
	{
		cursor_pos_ = cp;
		if (cursor_pos_ + first_column_ >= get_size().w)
		{
			first_column_ = cursor_pos_ - get_size().w;
		}
		request_redraw();
	}
}

void line_edit::home()
{
	if (cursor_pos_ != 0)
	{
		cursor_pos_ = 0;
		first_column_ = 0;
		request_redraw();
	}
}

void line_edit::render(ncurses_window& surface)
{
	style help_text_style = {style_.bgcolor, help_text_color_};

	// background
	surface.move_cursor(0, 0);
	surface.style_fill_line(style_, ' ', get_size().w);

	// text
	surface.move_cursor(0, 0);
	if (text_.empty())
	{
		surface.style_print(help_text_style, help_text_);
	}
	else if (first_column_ < text_.size())
	{
		surface.style_print(style_, text_.c_str() + first_column_, get_size().w);
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

	request_redraw();
}

void line_edit::hints_changed()
{
	if (hints_.empty())
	{
		hints_widget_.reset();
	}
	else
	{
		if (is_active() && is_visible())
		{
			show_hints();
		}
	}
}

void line_edit::show_hints()
{
	if (hints_.empty()) return;

	if (!hints_widget_.is_initialized())
	{
		hints_widget_.emplace(get_window_manager(), this);
	}

	// create hitns
	std::vector<list_widget::list_item> items;
	items.reserve(hints_.size());
	std::transform(hints_.begin(), hints_.end(), std::back_inserter(items),
			[&](const completion_hint& hint)
			{
				return list_widget::list_item{hint.text, hint.help_text};
			});

	hints_widget_->set_items(items);
	hints_widget_->set_filter(text_);
	update_hints_size();

	hints_widget_->show();
}

void line_edit::update_hints_size()
{
	assert(hints_widget_);

	size content_size = hints_widget_->get_content_size();

	position global_pos = to_global(position{0, 0});

	size sz;
	position pos;
	if (global_pos.y > LINES/2)
	{
		// bottom half of the screen - show hints above
		int max_h = std::min(global_pos.y-1, 40);
		int max_w = std::min(COLS - global_pos.x, 80);

		sz.h = std::min(max_h, content_size.h);
		sz.w = std::min(max_w, content_size.w);
		pos = {- sz.h, 0};
	}
	else
	{
		// top part of the screen, show hints below
		int max_h = std::min(LINES - global_pos.y, 40);
		int max_w = std::min(COLS - global_pos.x, 80);

		sz.h = std::min(max_h, content_size.h);
		sz.w = std::min(max_w, content_size.w);
		pos = {1, 0};
	}

	hints_widget_->move(pos, sz);
}



}
