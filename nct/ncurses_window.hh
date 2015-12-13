#pragma once
#include "ncurses_inc.hh"

#include "types.hh"

#include <string>

namespace nct {

class ncurses_window
{
public:

	ncurses_window(int height, int width, int starty, int startx);
	ncurses_window(WINDOW* w);
	~ncurses_window();

	WINDOW* get_window() const { return win_; }

	// output

	void print(const char* text) { ::wprintw(win_, text); }
	void print(const char* text, unsigned length) {::waddnstr(win_, text, length);}
	void print(const std::string& s) { print(s.c_str()); }
	void attr_print(attr_t attr, const char* text, unsigned length);
	void attr_print(attr_t attr, const std::string& s) { attr_print(attr, s.c_str(), s.length()); }
	void put_char(chtype c) { ::waddch(win_, c); }
	void attr_fill_line(attr_t attr, chtype c, int line);

	void clear() { ::wclear(win_); }
	void clear_to_eol() { ::wclrtoeol(win_); }
	void move_cursor(int row, int col) { ::wmove(win_, row, col); }
	void move_cursor(const position& pos) { ::wmove(win_, pos.y, pos.x); }

	// redraw/refresh

	void refresh() { ::wrefresh(win_); }
	void redraw() { ::redrawwin(win_); }
	void no_out_refresh() { ::wnoutrefresh(win_); }

	// getch

	int get_ch() { return ::wgetch(win_); }

	// attributes

	void set_attr_on(attr_t a) { ::wattron(win_, a); }
	void set_attr_off(attr_t a) { ::wattroff(win_, a); }
	void set_background(const chtype ch) { ::wbkgd(win_, ch); }

	// windows position/size

	void resize(int h, int w) { ::wresize(win_, h, w); }
	void move(int y, int x) { ::mvwin(win_, y, x); }

	int get_width() const { return getmaxx(win_); }
	int get_height() const { return getmaxy(win_); }

	size get_size() const { return size{get_height(), get_width()}; }

	// lines and borders

	void horizontal_line(int y, int x, chtype c, int len) { mvwhline(win_, y, x, c, len); }
	void horizontal_line(int y, int x, const cchar_t* c, int len) { mvwhline_set(win_, y, x, c, len); }

private:

	WINDOW* win_;
};

}
