#include <ncurses.h>

#include "ncurses_env.hh"
#include "editor.hh"
#include "styles.hh"

#include "document_lib/document.hh"

#include <iostream>

int main(int argc, char** argv)
{
	cpped::document::document document;
	if (argc > 1)
	{
		document.load_from_file(argv[1]);
	}
	document.parse_language();

	cpped::ncurses_env env;
	auto ss = env.get_stdscr();


	cpped::style_manager styles;
	cpped::editor editor(ss, document, styles);
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
