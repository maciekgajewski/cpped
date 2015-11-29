#include <ncurses.h>

#include "ncurses_env.hh"
#include "editor.hh"
#include "document_lib/document.hh"

#include <iostream>

int main(int argc, char** argv)
{
	cpped::ncurses_env env;
	auto ss = env.get_stdscr();

	cpped::document::document document;
	if (argc > 1)
	{
		document.load_from_file(argv[1]);
	}

	cpped::editor editor(ss, document);
	editor.render();

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
