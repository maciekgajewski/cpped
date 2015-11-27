#pragma once

#include <ncurses.h>

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
	void move(int row, int col) { ::wmove(win, row, col); }
	void set_attr(attr_t a) { ::wattron(win, a); }
	void unset_attr(attr_t a) { ::wattroff(win, a); }

	void color_printf(NCURSES_COLOR_T bg, NCURSES_COLOR_T fg, const char* fmt, ...) __attribute__ ((format (printf, 4, 5)));
	void color_print(NCURSES_COLOR_T bg, NCURSES_COLOR_T fg, const char* text);

	int get_ch() { ::wgetch(win); }
	int get_width() const { return getmaxx(win); }
	int get_height() const { return getmaxy(win); }

private:

	WINDOW* win;
};

}
