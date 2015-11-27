#include <ncurses.h>

#include "ncurses_env.hh"

namespace cpped {

ncurses_env::ncurses_env()
{
	::initscr();
	::raw();
	::noecho();
	::keypad(::stdscr, TRUE);
	::start_color();
	::mousemask(ALL_MOUSE_EVENTS|REPORT_MOUSE_POSITION, nullptr);
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
