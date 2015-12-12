#pragma once

#include "event_window.hh"

#include <string>

namespace nct {


// line edit widget
class line_edit : public event_window
{
public:

	line_edit(event_dispatcher& ed, event_window* parent);

private:

	void on_shown() override;

	void render();

	std::string text_;
	unsigned cursor_pos_ = 0;
	unsigned first_column_ = 0;
};

}
