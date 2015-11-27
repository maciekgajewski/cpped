#include "ncurses_env.hh"

#include <iostream>

int main()
{
	cpped::ncurses_env env;
	auto ss = env.get_stdscr();

	ss.print("Hello, world");
	ss.refresh();
	ss.get_ch();
}
