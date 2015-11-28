#include "document.hh"

#include "ncurses_window.hh"

#include <stdexcept>
#include <fstream>
#include <cassert>
#include <cmath>

namespace cpped {

static const int LEFT_BAR_WDTH_MARGIN = 3; // may differ in differnt mode/language

int document::left_bar_width() const
{
	return LEFT_BAR_WDTH_MARGIN + left_bar_digits;
}

int document::line_length(int line) const
{
	assert(line >= 0);

	if (line < data.size())
		return data[line].length();
	else if (line == data.size())
		return 0;
	else
		throw std::logic_error("Accessing line out of range");
}

void document::render(ncurses_window& window, int first_line, int first_column, int height, int width)
{
	int lines_to_render = std::min(height, get_lines() - first_line);

	char fmt[32];
	std::snprintf(fmt, 32, " %%%dd  ", left_bar_digits);

	int line = 0;
	for(; line < lines_to_render; ++line)
	{
		window.move(line, 0);
		const std::string& d = data[first_line+line];

		// render left bar with line number
		window.color_printf(COLOR_BLACK, COLOR_RED, fmt, line+first_line+1);

		// render text
		if (d.length() > first_column)
			window.print(d.c_str() + first_column);
	}

	// redner last line
	if (line < height)
	{
		window.move(line, 0);
		window.color_printf(COLOR_BLACK, COLOR_RED, fmt, line+first_line+1);
		window.color_print(COLOR_BLACK, COLOR_BLUE, "¶");
	}
}

void document::load_from_file(const std::__cxx11::string& path)
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
	left_bar_digits = int(std::ceil(std::log10(data.size()+1)));
}


}
