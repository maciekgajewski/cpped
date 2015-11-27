#pragma once

#include "ncurses_window.hh"

namespace cpped {

class ncurses_env
{
public:
	ncurses_env();
	~ncurses_env();

	ncurses_window get_stdscr();
};

}
