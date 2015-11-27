#include <ncurses.h>

#include "editor.hh"
#include "ncurses_window.hh"

#include <string>
#include <sstream>

namespace cpped {

editor::editor(ncurses_window& win)
	: window(win)
{
}

void editor::on_key(int key)
{
	std::ostringstream ss;
	ss << "key=" << key;
	window.clear();
	window.print(ss.str().c_str());
}

void editor::on_mouse(const MEVENT& event)
{
	std::ostringstream ss;
	ss << "mouse x=" << event.x << ", y=" << event.y << ", buttons=" << event.bstate;
	window.clear();
	window.print(ss.str().c_str());
}



}
