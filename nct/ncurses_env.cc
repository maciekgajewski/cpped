#include "ncurses_inc.hh"

#include "ncurses_env.hh"

#include <stdexcept>

namespace nct {

ncurses_env* ncurses_env::current_ = nullptr;

ncurses_env::ncurses_env()
{
	if (current_)
		throw std::logic_error("There can be only one!");

	current_ = this;

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
	current_ = nullptr;
}

ncurses_window ncurses_env::get_stdscr()
{
	return stdscr;
}

}
