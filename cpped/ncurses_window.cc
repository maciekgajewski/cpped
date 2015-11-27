#include <ncurses.h>

#include "ncurses_window.hh"

#include <cstdarg>

namespace cpped {

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

void ncurses_window::color_printf(NCURSES_COLOR_T bg, NCURSES_COLOR_T fg, const char* fmt, ...)
{
	std::va_list args;
	va_start(args, fmt);

	::init_pair(1, fg, bg);
	::wattron(win, COLOR_PAIR(1));
	::vwprintw(win, fmt, args);
	::wattroff(win, COLOR_PAIR(1));

	va_end(args);
}

}
