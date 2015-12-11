#pragma once

#include "ncurses_window.hh"

namespace nct {

class ncurses_env
{
public:
	ncurses_env();
	~ncurses_env();

	ncurses_window get_stdscr();
};

}
