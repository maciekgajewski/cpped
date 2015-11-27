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

	// action handlers
	void cursor_up();
	void cursor_down();
	void cursor_left();
	void cursor_right();

	void scroll_down();
	void scroll_up();
	void scroll_left();
	void scroll_right();

	void move_cursor(int y, int x);

	int left_bar_width() const;
	int current_line_length() const;
	int workspace_width() const;

	std::vector<std::string> data;
	int first_line = 0;
	int first_column = 0;
	int left_bar_digits = 1;

	// cursor's screen pos
	int cursor_x = 0;
	int cursor_y = 0;
	int desired_cursor_x = 0;

	ncurses_window& window;
};

}
