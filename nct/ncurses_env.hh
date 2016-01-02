#pragma once

#include "ncurses_window.hh"
#include "style.hh"

namespace nct {

class ncurses_env
{
public:
	ncurses_env();
	~ncurses_env();

	ncurses_window get_stdscr();

	static ncurses_env* get_current() { return current_; }

	int style_to_attr(const style& s);

private:

	static ncurses_env* current_;
	color_palette palette_;
};

}
