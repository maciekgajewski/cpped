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
	void print(const char* text) { ::wprintw(win, text); }
	void refresh() { ::wrefresh(win); }
	void clear() { ::wclear(win); }

	int get_ch() { ::wgetch(win); }
	int get_max_x() const { return getmaxx(win); }
	int get_max_y() const {	return getmaxy(win); }

private:

	WINDOW* win;
};

}
