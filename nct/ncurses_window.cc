#include <ncursesw/ncurses.h>

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
	::wattroff(win_, attr);
}

}
