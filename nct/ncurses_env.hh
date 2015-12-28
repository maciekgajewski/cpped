#pragma once

#include "ncurses_window.hh"

namespace nct {

class ncurses_env
{
public:
	ncurses_env();
	~ncurses_env();

	ncurses_window get_stdscr();

	static ncurses_env* get_current() { return current_; }

private:

	static ncurses_env* current_;
};

}
