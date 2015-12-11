#pragma once

#include "ncurses_window.hh"

#include <string>

namespace nct {

class event_dispatcher;

class event_window : public ncurses_window
{
public:
	event_window(WINDOW* w, event_dispatcher& ed, int z, event_window* parent);
	event_window(int h, int w, int y, int x, event_dispatcher& ed, int z, event_window* parent);
	virtual ~event_window();

	// event receviers

	/// When a sequnece of pritnable character has been recevied
	/// returns: number of characters consumed
	virtual unsigned on_sequence(const std::string& s) { return 0; }

	// When special key was presed
	// return: true if key was accepted
	virtual bool on_special_key(int key_code, const char* key_name) { return false; }

	/// mouse handler
	virtual void on_mouse(const MEVENT& event) {}

	event_window* get_parent() const { return parent_; }

	void set_active();

private:

	event_dispatcher& event_dispatcher_;
	int z_;
	event_window* parent_;
};

}
