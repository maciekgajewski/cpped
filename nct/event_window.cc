#include "event_window.hh"

#include "event_dispatcher.hh"

namespace nct {

event_window::event_window(WINDOW* w, event_dispatcher& ed, int z, event_window* parent)
	: ncurses_window(w), event_dispatcher_(ed), z_(z), parent_(parent)
{
	event_dispatcher_.add_window(this);
}

event_window::event_window(int h, int w, int y, int x, event_dispatcher& ed, int z, event_window* parent)
	: ncurses_window(h, w, y, x), event_dispatcher_(ed), z_(z), parent_(parent)
{
	event_dispatcher_.add_window(this);
}

event_window::~event_window()
{
	event_dispatcher_.remove_window(this);
}

void event_window::set_active()
{
	event_dispatcher_.set_active_window(this);
}

}
