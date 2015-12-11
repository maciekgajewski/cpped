#pragma once

#include <ncursesw/ncurses.h> // only for key codes and MEVENT

#include "document_lib/document.hh"

#include <string>

namespace cpped {

class editor_window;

class editor
{
public:
	editor(editor_window& win, document::document& d);

	bool on_special_key(int key_code, const char* key_name);
	unsigned on_sequence(const std::string& sequence);
	bool on_mouse(const MEVENT& event);

private:

	// action handlers
	void cursor_up();
	void cursor_down();
	void cursor_left();
	void cursor_right();

	void pg_up();
	void pg_down();

	void scroll_down();
	void scroll_up();
	void scroll_left();
	void scroll_right();

	void backspace();
	void del();

	// workspace/doc coordinates
	int column_to_workspace_x(unsigned column) const;
	int documet_to_workspace_y(unsigned docy) const;
	unsigned workspace_to_document_x(unsigned wx) const;
	unsigned workspace_to_document_y(unsigned wy) const;
	void adjust_cursor_column_to_desired();

	void ensure_cursor_visible();

	void request_full_render();
	void request_cursor_update();

	// converts document x (character in line) into column, taking all tabs into account
	unsigned document_x_to_column(unsigned docy, unsigned docx) const;

	// manipulation
	void insert_at_cursor(const std::string& s);

	unsigned first_line_ = 0;
	unsigned first_column_ = 0;

	// cursor document poisition
	document::position cursor_pos_ = {0, 0};
	unsigned desired_cursor_column_ = 0;

	// settings
	unsigned tab_width_ = 4;

	document::document& doc_;
	editor_window& window_;
};

}
