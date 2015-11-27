#pragma once

#include <ncurses.h>

#include <vector>
#include <string>

namespace cpped {

class ncurses_window;

class editor
{
public:
	editor(ncurses_window& win);

	void on_key(int key);
	void on_mouse(const MEVENT& event);
	void load_from_file(const std::string& path);
	void render();

private:


	std::vector<std::string> data;
	int first_line = 0;
	int first_column = 0;
	int left_bar_digits = 1;

	ncurses_window& window;
};

}
