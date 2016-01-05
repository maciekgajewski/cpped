#pragma once

#include "event_window.hh"

namespace nct {

// Line with buttons associated with F keys. Think: Norton Commander, Midnight Commander, HTop
//
// There can be only one instance. Other widgets may control the text on the buttons using fbuttons_provider
class fbuttons : public event_window
{
public:

	fbuttons(event_dispatcher& ed, event_window* parent);

private:

	void on_shown() override { update(); }

	void update();
	std::string get_text(unsigned idx) const;
};

}
