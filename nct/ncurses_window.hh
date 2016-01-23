#pragma once
#include "ncurses_inc.hh"

#include "types.hh"
#include "style.hh"

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
	void put_char(const style& s, chtype c) { ::waddch(win_, c | s.to_attr()); }
	void attr_fill_line(attr_t attr, chtype c, int line);

	// prints character 'count' times at current position
	void attr_fill(attr_t attr, chtype c, int count);

	void style_print(const style& s, const std::string& text) { style_print(s, text.c_str(), text.size()); }
	void style_print(const style& s, const char* text, unsigned length);
	void style_fill_line(const style& s, chtype c, int line) { attr_fill_line(s.to_attr(), c, line); }
	void style_fill(const style& s, chtype c, int count) { attr_fill(s.to_attr(), c, count); }

	// Advanced printing - no wrapping etc
	void print(const position& pos, const style& s, const std::string& text);

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
	void set_background(const style& s, const chtype ch) { ::wbkgd(win_, s.to_attr() | ch); }
	void set_style_on(const style& s) { ::wattron(win_, s.to_attr()); }
	void set_style_off(const style& s) { ::wattroff(win_, s.to_attr()); }

	// windows position/size

	void resize(int h, int w) { ::wresize(win_, h, w); }
	void move(int y, int x) { ::mvwin(win_, y, x); }

	int get_width() const { return getmaxx(win_); }
	int get_height() const { return getmaxy(win_); }

	size get_size() const { return size{get_height(), get_width()}; }

	// lines and borders

	void horizontal_line(int y, int x, chtype c, int len) { mvwhline(win_, y, x, c, len); }
	void horizontal_line(int y, int x, style& s, chtype c, int len) { mvwhline(win_, y, x, s.to_attr() | c, len); }
	void horizontal_line(int y, int x, const cchar_t* c, int len) { mvwhline_set(win_, y, x, c, len); }

	void vertical_line(int y, int x, chtype c, int len) { mvwvline(win_, y, x, c, len); }

private:

	WINDOW* win_;
};

}
