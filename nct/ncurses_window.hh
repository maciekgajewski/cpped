#pragma once

#include <ncursesw/ncurses.h>

namespace nct {

class ncurses_window
{
public:

	ncurses_window(int height, int width, int starty, int startx);
	ncurses_window(WINDOW* w);
	~ncurses_window();
	void print(const char* text) { ::wprintw(win, text); }
	void print(const char* text, unsigned length) {::waddnstr(win, text, length);}
	void attr_print(attr_t attr, const char* text, unsigned length);
	void put_char(chtype c) { ::waddch(win, c); }
	void refresh() { ::wrefresh(win); }
	void clear() { ::wclear(win); }
	void clear_to_eol() { ::wclrtoeol(win); }
	void move(int row, int col) { ::wmove(win, row, col); }
	void set_attr(attr_t a) { ::wattron(win, a); }
	void unset_attr(attr_t a) { ::wattroff(win, a); }


	int get_ch() { return ::wgetch(win); }
	int get_width() const { return getmaxx(win); }
	int get_height() const { return getmaxy(win); }

	void set_attr_on(attr_t a) { ::wattron(win, a); }
	void set_attr_off(attr_t a) { ::wattroff(win, a); }

private:

	WINDOW* win;
};

}