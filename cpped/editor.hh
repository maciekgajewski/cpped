#pragma once

#include <ncursesw/ncurses.h>


namespace cpped {

namespace document{
	class document;
}

class editor_window;

class editor
{
public:
	editor(editor_window& win, document::document& d);

	void on_key(int key);
	void on_mouse(const MEVENT& event);

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

	// workspace/doc coordinates
	int column_to_workspace_x(unsigned column) const;
	int documet_to_workspace_y(unsigned docy) const;
	unsigned workspace_to_document_x(unsigned wx) const;
	unsigned workspace_to_document_y(unsigned wy) const;
	void adjust_cursor_column_to_desired(unsigned new_line_len);

	void request_full_render();
	void request_cursor_update();

	// converts document x (character in line) into column, taking all tabs into account
	unsigned document_x_to_column(unsigned docy, unsigned docx) const;

	// manipulation
	void insert_at_cursor(char c);

	unsigned first_line_ = 0;
	unsigned first_column_ = 0;

	// cursor document poisition
	unsigned cursor_x_ = 0;
	unsigned cursor_y_ = 0;
	unsigned desired_cursor_column_ = 0;

	// settings
	unsigned tab_width_ = 4;

	document::document& doc_;
	editor_window& window_;
};

}
