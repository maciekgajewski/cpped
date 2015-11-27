#include <ncurses.h>

#include "ncurses_env.hh"

namespace cpped {

ncurses_env::ncurses_env()
{
	::initscr();
	::raw();
	::noecho();
	::keypad(::stdscr, TRUE);
}

ncurses_env::~ncurses_env()
{
	::endwin();
}

ncurses_window ncurses_env::get_stdscr()
{
	return stdscr;
}

}
