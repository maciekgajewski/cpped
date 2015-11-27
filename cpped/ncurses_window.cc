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

	int pair = fg +(bg<<3);
	::init_pair(pair, fg, bg);
	::wattron(win, COLOR_PAIR(pair));
	::vwprintw(win, fmt, args);
	::wattroff(win, COLOR_PAIR(pair));

	va_end(args);
}

void ncurses_window::color_print(short bg, short fg, const char* text)
{
	int pair = fg +(bg<<3);
	::init_pair(pair, fg, bg);
	::wattron(win, COLOR_PAIR(pair));
	::wprintw(win, text);
	::wattroff(win, COLOR_PAIR(pair));
}

}
