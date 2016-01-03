#pragma once

#include <ncursesw/ncurses.h> // only for key codes and MEVENT

#include "document_lib/document.hh"

#include <string>

namespace cpped {

class editor_window;

struct editor_settings
{
	bool visualize_tab = true;
	unsigned tab_width = 4;
};

class editor
{
public:
	editor(editor_window& win, document::document& d);
	editor(editor_window& win);

	bool on_special_key(int key_code, const char* key_name);
	unsigned on_sequence(const std::string& sequence);
	bool on_mouse(const MEVENT& event);

	void update() { request_full_render(); }

	void set_document(document::document& doc);

	const document::document* get_document() const { return doc_; }
	document::document_position get_cursor_position() const { return cursor_pos_; }

	void disable_parsing() { parsing_disabled_ = true; }
	void enable_parsing() { parsing_disabled_ = false; }

	void replace(const document::document_position& pos, unsigned len, const std::string& replacement);

private:

	// action handlers
	void arrow_up();
	void arrow_down();
	void arrow_left();
	void arrow_right();

	void shift_arrow_up();
	void shift_arrow_down();
	void shift_arrow_left();
	void shift_arrow_right();

	void pg_up();
	void pg_down();

	void backspace();
	void del();

	void home();
	void end();

	void cursor_up();
	void cursor_down();
	void cursor_left();
	void cursor_right();

	void copy();
	void paste();
	void cut();

	template<typename Action>
	void shift_arrow(Action action);

	void on_document_tokens_updated();

	// workspace/doc coordinates
	int column_to_workspace_x(unsigned column) const;
	int documet_to_workspace_y(unsigned docy) const;
	unsigned workspace_to_document_x(unsigned wx) const;
	unsigned workspace_to_document_y(unsigned wy) const;
	void adjust_cursor_column_to_desired();

	void ensure_cursor_visible();

	void request_full_render();
	void request_cursor_update();
	void request_parsing();

	// converts document x (character in line) into column, taking all tabs into account
	unsigned document_x_to_column(unsigned docy, unsigned docx) const;

	// manipulation
	void insert_at_cursor(const std::string& s);

	unsigned first_line_ = 0;
	unsigned first_column_ = 0;

	// cursor document poisition
	document::document_position cursor_pos_ = {0, 0};
	unsigned desired_cursor_column_ = 0;

	// settings
	editor_settings settings_;

	document::document* doc_;
	editor_window& window_;
	std::unique_ptr<document::document> unsaved_document_;
	bool parsing_disabled_ = false;
	boost::optional<document::document_range> selection_;
	bool extending_selection_ = false;
};

}
