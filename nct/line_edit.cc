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

void line_edit::on_shown()
{
	render();
}

void line_edit::render()
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

}
