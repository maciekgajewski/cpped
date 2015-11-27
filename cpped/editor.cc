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
	std::ostringstream ss;
	ss << "key=" << key;
	window.clear();
	window.print(ss.str().c_str());
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
}

void editor::render()
{
	window.clear();

	int lines = std::min(window.get_height(), int(data.size()) - first_line);

	char fmt[32];
	std::snprintf(fmt, 32, " %%%dd  ", left_bar_digits);

	for(int line = 0; line < lines; ++line)
	{
		window.move(line, 0);
		const std::string& d = data[first_line+line];

		// render left bar with line number
		window.color_printf(COLOR_BLACK, COLOR_RED, fmt, line+first_line);

		// render text
		window.print(d.c_str());
	}
}



}
