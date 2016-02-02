#include "ncurses_inc.hh"

#include "ncurses_window.hh"

#include <cstdarg>

namespace nct {

ncurses_window::ncurses_window(int height, int width, int starty, int startx)
{
	win_ = ::newwin(height, width, starty, startx);
	::keypad(win_, TRUE);
	::leaveok(win_, FALSE);
	::wtimeout(win_, 0); // enter non-blocking mode
}

ncurses_window::ncurses_window(position pos, size sz)
	: ncurses_window(sz.h, sz.w, pos.y, pos.x)
{
}

ncurses_window::ncurses_window(WINDOW* w)
	:	win_(w)
{
}

ncurses_window::~ncurses_window()
{
	if (win_ != stdscr)
		::delwin(win_);
}

void ncurses_window::attr_print(attr_t attr, const char* text, unsigned length)
{
	::wattron(win_, attr);
	::waddnstr(win_, text, length);
	::wattroff(win_, attr);
}

void ncurses_window::attr_fill_line(attr_t attr, chtype c, int line)
{
	move_cursor(line, 0);
	attr_fill(attr, c, get_width());
}

void ncurses_window::attr_fill(attr_t attr, chtype c, int count)
{
	::wattron(win_, attr);

	for(int i = 0; i < count; i++)
		put_char(c);

	::wattron(win_, attr);
}

void ncurses_window::style_print(const style& s,  const char* text, unsigned length)
{
	int attr = s.to_attr();
	::wattron(win_, attr);
	::waddnstr(win_, text, length);
	::wattroff(win_, attr);
}

void ncurses_window::print(const position& pos, const style& s, const std::string& text)
{
	if (pos.y >= 0 && pos.y < get_height() && pos.x < get_width())
	{
		int chars_to_print = std::min<int>(
			text.length(), get_width() - pos.x);
		move_cursor(pos);
		style_print(s, text.c_str(), chars_to_print);
	}
}

}
