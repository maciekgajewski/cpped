#pragma once

#include "editor.hh"

#include <chrono>

namespace cpped {

class ncurses_window;
class style_manager;

namespace document{
	class document;
}

class editor_window
{
public:

	editor_window(ncurses_window& win, style_manager& sm, document::document& doc);

	void on_key(int key) { editor_.on_key(key); }
	void on_mouse(const MEVENT& event) { editor_.on_mouse(event); }

	void render(document::document& doc, unsigned first_column, unsigned first_line, unsigned tab_width);
	void update_status_line(unsigned docy, unsigned docx, unsigned column, std::chrono::high_resolution_clock::duration last_parse_time);
	void refresh_cursor(int wy, int wx);

	unsigned get_workspace_width() const;
	unsigned get_workspace_height() const;

private:

	// rendering
	unsigned render_text(attr_t attr, unsigned tab_width, unsigned first_column, unsigned phys_column, const char* begin, const char* end); // returns last physical column
	void put_visual_tab();

	unsigned left_margin_width_ = 0; // calculated when rendering
	unsigned top_margin_ = 0;
	unsigned bottom_margin_ = 1;

	// settings
	bool visualise_tabs_ = true;

	ncurses_window& window_;
	style_manager& styles_;
	editor editor_;
};

}
