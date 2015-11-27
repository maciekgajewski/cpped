#pragma once

#ifndef CURSES
struct WINDOW;
#endif

namespace cpped {

class ncurses_window
{
public:

	ncurses_window(int height, int width, int starty, int startx);
	ncurses_window(WINDOW* w);
	~ncurses_window();
	void print(const char* text);
	void refresh();

	int get_ch();

private:

	WINDOW* win;
};

}
