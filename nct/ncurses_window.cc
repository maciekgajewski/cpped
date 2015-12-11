#include <ncursesw/ncurses.h>

#include "ncurses_window.hh"

#include <cstdarg>

namespace nct {

ncurses_window::ncurses_window(int height, int width, int starty, int startx)
{
	win = ::newwin(height, width, starty, startx);
}

ncurses_window::ncurses_window(WINDOW* w)
	:	win(w)
{
}

ncurses_window::~ncurses_window()
{
	if (win != stdscr)
		::delwin(win);
}

void ncurses_window::attr_print(attr_t attr, const char* text, unsigned length)
{
	::wattron(win, attr);
	::waddnstr(win, text, length);
	::wattroff(win, attr);
}

}
