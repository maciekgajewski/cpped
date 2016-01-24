#pragma once

#include "event_window.hh"
#include "list_widget.hh"

#include <boost/signals2.hpp>

#include <string>

namespace nct {


// line edit widget
class line_edit : public event_window
{
public:

	line_edit(window_manager& ed, event_window* parent);

	void set_help_text(const std::string& t) { help_text_ = t; }
	void set_text(const std::string& t);
	void move_cursor_to_end();

	void set_style(const style& s) { style_ = s; }
	void set_help_text_color(int color) { help_text_color_ = color; }

// signals

	boost::signals2::signal<void(const std::string&)> text_changed_signal;
	boost::signals2::signal<void()> enter_pressed_signal;

private:

	unsigned on_sequence(const std::string& s) override;
	bool on_special_key(int key_code, const char* key_name) override;

	void on_shown() override;

	void render(ncurses_window& surface) override;

	void cursor_left();
	void cursor_right();
	void backspace();
	void del();
	void enter();
	void end();
	void home();

	void on_text_changed();

	std::string text_;
	std::string help_text_;
	unsigned cursor_pos_ = 0;
	unsigned first_column_ = 0;

	style style_;
	int help_text_color_;
};

}
