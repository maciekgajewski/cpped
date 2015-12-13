#pragma once

#include "event_window.hh"

#include <boost/signals2.hpp>

#include <string>

namespace nct {


// line edit widget
class line_edit : public event_window
{
public:

	struct completion_hint
	{
		const std::string text;
		const std::string help_text;
	};

	line_edit(event_dispatcher& ed, event_window* parent);

	void set_help_text(const std::string& t) { help_text_ = t; }
	void set_text(const std::string& t);

	template<typename Container>
	void set_completion_hints(const Container& hints);

// signals

	boost::signals2::signal<void(const std::string&)> text_changed;
	boost::signals2::signal<void()> enter_pressed;

private:

	unsigned on_sequence(const std::string& s) override;
	bool on_special_key(int key_code, const char* key_name) override;

	void on_shown() override;

	void cursor_left();
	void cursor_right();
	void backspace();
	void del();

	void update();

	void hints_changed();

	std::string text_;
	std::string help_text_;
	unsigned cursor_pos_ = 0;
	unsigned first_column_ = 0;

	std::vector<completion_hint> hints_;
};

template<typename Container>
void line_edit::set_completion_hints(const Container& hints)
{
	hints_.assign(std::begin(hints), std::end(hints));
	hints_changed();
}


}
