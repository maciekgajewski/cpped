#include <ncurses.h>

#include "ncurses_window.hh"

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

}
