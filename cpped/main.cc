#include <ncurses.h>

#include "ncurses_env.hh"
#include "editor.hh"

#include <iostream>

int main()
{
	cpped::ncurses_env env;
	auto ss = env.get_stdscr();

	//cpped::ncurses_window win(50, 50, 0, 0);
	cpped::editor editor(ss);

	while(true)
	{
		int key = ::getch();
		if (key == 'q')
			break;
		MEVENT mouse_event;
		if(::getmouse(&mouse_event) == OK)
			editor.on_mouse(mouse_event);
		else
			editor.on_key(key);
	}
}
