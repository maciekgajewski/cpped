#include <ncurses.h>

#include "editor.hh"
#include "ncurses_window.hh"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cmath>

namespace cpped {

static const int LEFT_BAR_WDTH_MARGIN = 3;

editor::editor(ncurses_window& win)
	: window(win)
{
}

void editor::on_key(int key)
{
//	std::ostringstream ss;
//	ss << "key=" << key;
//	window.clear();
//	window.print(ss.str().c_str());

	// hard-coded actions
	switch(key)
	{
		case KEY_UP:
			cursor_up();
			break;
		case KEY_DOWN:
			cursor_down();
			break;
		case KEY_LEFT:
			cursor_left();
			break;
		case KEY_RIGHT:
			cursor_right();
			break;
	}
}

void editor::on_mouse(const MEVENT& event)
{
	std::ostringstream ss;
	ss << "mouse x=" << event.x << ", y=" << event.y << ", buttons=" << event.bstate;
	window.clear();
	window.print(ss.str().c_str());
}

void editor::load_from_file(const std::__cxx11::string& path)
{
	std::fstream f(path, std::ios_base::in);
	if (f.fail())
	{
		throw std::runtime_error("Error opening file");
	}

	data.clear();

	std::string line;
	while(!f.eof())
	{
		line.clear();
		std::getline(f, line);
		data.push_back(line);
	}
	first_line = 0;
	first_column = 0;

	left_bar_digits = int(std::ceil(std::log10(data.size()+1)));
	move_cursor(0,0);
}

void editor::render()
{
	window.clear();

	int lines = std::min(window.get_height(), int(data.size()) - first_line);

	char fmt[32];
	std::snprintf(fmt, 32, " %%%dd  ", left_bar_digits);

	int line = 0;
	for(; line < lines; ++line)
	{
		window.move(line, 0);
		const std::string& d = data[first_line+line];

		// render left bar with line number
		window.color_printf(COLOR_BLACK, COLOR_RED, fmt, line+first_line+1);

		// render text
		window.print(d.c_str());
	}

	// redner last line
	if (line < window.get_height())
	{
		window.move(line, 0);
		window.color_printf(COLOR_BLACK, COLOR_RED, fmt, line+first_line+1);
		window.color_print(COLOR_BLACK, COLOR_BLUE, "¶");
	}

	// restore cursor
	move_cursor(cursor_y, cursor_x);
}

void editor::cursor_up()
{
	if (cursor_y == 0)
	{
		scroll_up();
	}
	else
	{
		move_cursor(cursor_y - 1, cursor_x);
	}
}

void editor::cursor_down()
{
	if (cursor_y == window.get_height()-1)
	{
		scroll_down();
	}
	else
	{
		if (cursor_y < data.size() - first_line)
		{
			move_cursor(cursor_y + 1, cursor_x);
		}
	}
}

void editor::cursor_left()
{
	// TODO
}

void editor::cursor_right()
{
	// TODO
}

void editor::scroll_down()
{
	if (first_line + window.get_height() < data.size() + 1)
	{
		++first_line;
		render();
	}
}

void editor::scroll_up()
{
	if (first_line > 0)
	{
		--first_line;
		render();
	}
}

void editor::move_cursor(int y, int x)
{
	cursor_y = y;
	cursor_x = x;
	window.move(y, x + left_bar_width() + 1);
}

int editor::left_bar_width() const
{
	return left_bar_digits + LEFT_BAR_WDTH_MARGIN;
}



}
