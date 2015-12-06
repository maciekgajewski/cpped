#include "event_window.hh"

#include "event_dispatcher.hh"

namespace cpped {

event_window::event_window(cpped::event_dispatcher& ed, cpped::ncurses_window& window, int z, event_window* parent)
	: event_dispatcher_(ed), window_(window), z_(z), parent_(parent)
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
