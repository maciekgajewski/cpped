#pragma once

#include <ncursesw/ncurses.h>


namespace cpped {

class ncurses_window;
class style_manager;

namespace document{
	class document;
}

class editor
{
public:
	editor(ncurses_window& win, document::document& d, style_manager& sm);

	void on_key(int key);
	void on_mouse(const MEVENT& event);
	void render();
	void set_document(document::document& doc);

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

	void refresh_cursor();

	// rendering
	unsigned render_text(attr_t attr, unsigned phys_column, const char* begin, const char* end); // returns last physical column
	void put_visual_tab();
	void update_status_line();

	// workspace/doc coordinates
	unsigned get_workspace_width() const;
	int get_workspace_height() const;
	int column_to_workspace_x(unsigned column) const;
	int documet_to_workspace_y(unsigned docy) const;
	unsigned workspace_to_document_x(unsigned wx) const;
	unsigned workspace_to_document_y(unsigned wy) const;
	void adjust_cursor_column_to_desired(unsigned new_line_len);

	// converts document x (character in line) into column, taking all tabs into account
	unsigned document_x_to_column(unsigned docy, unsigned docx) const;

	// manipulation
	void insert_at_cursor(char c);

	unsigned first_line = 0;
	unsigned first_column = 0;
	unsigned left_margin_width = 0; // calculated when rendering
	unsigned top_margin = 0;
	unsigned bottom_margin = 1;

	// cursor's screen pos
	unsigned cursor_doc_x = 0;
	unsigned cursor_doc_y = 0;
	unsigned desired_cursor_column = 0;

	// settings
	unsigned tab_width = 4;
	bool visualise_tabs = true;

	ncurses_window& window;
	document::document* doc;
	style_manager& styles;
};

}
