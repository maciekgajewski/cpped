#include "line_edit.hh"

#include "event_dispatcher.hh"

namespace nct {

line_edit::line_edit(event_dispatcher& ed, event_window* parent)
	: event_window(ed, parent)
{
}

void line_edit::on_shown()
{
	render();
}

void line_edit::render()
{
	if (!is_visible()) return;
	ncurses_window& window = get_ncurses_window();

	int attr = get_palette().get_pair_for_colors(COLOR_WHITE, COLOR_BLACK);
	window.horizontal_line(0, 0, ' ', get_size().w);
	window.move_cursor(0, 0);
	window.set_attr_on(attr);
	if (first_column_ < text_.size())
	{
		window.print(text_.c_str() + first_column_, get_size().w);
	}

	if (cursor_pos_ >= first_column_)
	{
		show_cursor({0, int(cursor_pos_-first_column_)});
	}
	else
	{
		hide_cursor();
	}


}

}
