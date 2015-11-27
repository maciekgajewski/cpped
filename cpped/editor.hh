#pragma once

#include <ncurses.h>

namespace cpped {

class ncurses_window;

class editor
{
public:
	editor(ncurses_window& win);

	void on_key(int key);
	void on_mouse(const MEVENT& event);

private:

	ncurses_window& window;
};

}
