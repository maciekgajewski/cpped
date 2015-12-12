#pragma once

#include <ncursesw/ncurses.h>

#include "types.hh"

namespace nct {

class ncurses_window
{
public:

	ncurses_window(int height, int width, int starty, int startx);
	ncurses_window(WINDOW* w);
	~ncurses_window();

	WINDOW* get_window() const { return win_; }

	void print(const char* text) { ::wprintw(win_, text); }
	void print(const char* text, unsigned length) {::waddnstr(win_, text, length);}
	void attr_print(attr_t attr, const char* text, unsigned length);
	void put_char(chtype c) { ::waddch(win_, c); }
	void refresh() { ::wrefresh(win_); }
	void redraw() { ::redrawwin(win_); }
	void no_out_refresh() { ::wnoutrefresh(win_); }
	void clear() { ::wclear(win_); }
	void clear_to_eol() { ::wclrtoeol(win_); }
	void move_cursor(int row, int col) { ::wmove(win_, row, col); }
	void set_attr(attr_t a) { ::wattron(win_, a); }
	void unset_attr(attr_t a) { ::wattroff(win_, a); }


	int get_ch() { return ::wgetch(win_); }
	int get_width() const { return getmaxx(win_); }
	int get_height() const { return getmaxy(win_); }

	size get_size() const { return size{get_height(), get_width()}; }

	void set_attr_on(attr_t a) { ::wattron(win_, a); }
	void set_attr_off(attr_t a) { ::wattroff(win_, a); }

	void resize(int h, int w) { ::wresize(win_, h, w); }
	void move(int y, int x) { ::mvwin(win_, y, x); }

private:

	WINDOW* win_;
};

}
