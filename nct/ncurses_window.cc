#include "ncurses_inc.hh"

#include "ncurses_window.hh"

#include <cstdarg>

namespace nct {

ncurses_window::ncurses_window(int height, int width, int starty, int startx)
{
	win_ = ::newwin(height, width, starty, startx);
	::keypad(win_, TRUE);
}

ncurses_window::ncurses_window(WINDOW* w)
	:	win_(w)
{
}

ncurses_window::~ncurses_window()
{
	if (win_ != stdscr)
		::delwin(win_);
}

void ncurses_window::attr_print(attr_t attr, const char* text, unsigned length)
{
	::wattron(win_, attr);
	::waddnstr(win_, text, length);
	::wattron(win_, attr);
}

void ncurses_window::attr_fill_line(attr_t attr, chtype c, int line)
{
	move_cursor(line, 0);

	::wattron(win_, attr);

	for(int i = 0; i < get_width(); i++)
		put_char(c);

	::wattron(win_, attr);
}

}
