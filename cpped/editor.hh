#pragma once

#include <ncurses.h>


namespace cpped {

class ncurses_window;

namespace document{
	class document;
}

class editor
{
public:
	editor(ncurses_window& win, document::document& d);

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

	// workspace/doc coordinates
	int get_workspace_width() const;
	int get_workspace_height() const;
	int documet_to_workspace_x(int docx) const;
	int documet_to_workspace_y(int docy) const;
	int workspace_to_document_x(int wx) const;
	int workspace_to_document_y(int wy) const;

	int first_line = 0;
	int first_column = 0;
	int left_margin_width = 0; // calculated when rendering

	// cursor's screen pos
	int cursor_doc_x = 0;
	int cursor_doc_y = 0;
	int desired_cursor_x = 0;

	ncurses_window& window;
	document::document* doc;
};

}
