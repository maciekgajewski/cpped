#pragma once

#include <ncursesw/ncurses.h> // only for key codes and MEVENT

#include "document_lib/document.hh"

#include "nct/types.hh"

#include <string>

namespace cpped {

class editor_window;
class edited_file;

struct editor_settings
{
	bool visualize_tab = true;
	unsigned tab_width = 4;
};

class editor
{
public:
	struct status_info
	{
		unsigned docy, docx, column; // current cursor coordinates
		std::string status_text; // extra text displayed in status line
		boost::filesystem::path file_name;
		bool unsaved; // if file has unsaved changes
	};

	editor(editor_window& win, edited_file& f);

	bool on_special_key(int key_code, const char* key_name);
	unsigned on_sequence(const std::string& sequence);
	bool on_mouse(const MEVENT& event);

	void update() { request_full_render(); }

	void set_document(edited_file& f);

	const document::document& get_document() const;
	document::document& get_document();

	edited_file& get_file() { assert(file_); return *file_; }
	const edited_file& get_file() const { assert(file_); return *file_; }

	document::document_position get_cursor_position() const { return cursor_pos_; }

	void disable_parsing() { parsing_disabled_ = true; }
	void enable_parsing() { parsing_disabled_ = false; }

	void replace(const document::document_position& pos, unsigned len, const std::string& replacement);

	unsigned get_first_line() const { return first_line_; }
	unsigned get_first_column() const { return first_column_; }
	boost::optional<document::document_range> get_selection() const { return selection_; }
	const editor_settings& get_settings() const { return settings_; }
	status_info get_status_info() const;
	// return cursor position in workspace coords
	nct::position get_cursor_workspace_position() const;

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
	void request_parsing();

	void update_window_title();

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

	edited_file* file_ = nullptr;
	editor_window& window_;
	bool parsing_disabled_ = false;
	boost::optional<document::document_range> selection_;
	bool extending_selection_ = false;
	boost::signals2::scoped_connection tokens_udated_connection_;
};

}
