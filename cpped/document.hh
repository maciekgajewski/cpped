#pragma once

#include <vector>
#include <string>

namespace cpped {

class ncurses_window;

class document
{
public:

	void load_from_file(const std::string& path);

	int get_lines() const { return data.size(); }
	int left_bar_width() const;
	int line_length(int line) const;

	void render(ncurses_window& win, int first_line, int first_column, int height, int width);

private:

	void parse();

	int left_bar_digits = 1;

	std::vector<std::string> data;
};

}
