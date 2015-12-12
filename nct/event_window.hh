#pragma once

#include "ncurses_window.hh"
#include "types.hh"

#include <boost/optional.hpp>

#include <string>

namespace nct {

class event_dispatcher;

class event_window
{
public:
	event_window(event_dispatcher& ed, event_window* parent);
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

	// window shown/hidden
	virtual void on_shown() {}
	virtual void on_hidden() {}

	event_window* get_parent() const { return parent_; }

	// window state manipulation

	void set_active();

	void hide();
	void show();
	bool is_visible() const { return window_.is_initialized(); }

	// window parameters
	// position is always relative to the parent

	const size& get_size() const { return size_; }
	const position& get_position() const { return position_; }

	void move(const position& pos, const size& sz);
	void set_size(const size& sz) { move(position_, sz); }
	void set_position(const position& pos) { move(pos, size_); }

	// Returns reference to ncurses window, throws if hidden and ncurses window not created.
	// note: the reference may be invalidated by call to hide() or another method. Do not store!
	ncurses_window& get_ncurses_window();

protected:

	// converts local to global (screen) coordinates
	position to_global(const position& pos);

private:

	event_dispatcher& event_dispatcher_;
	boost::optional<ncurses_window> window_;
	int z_ = 0;
	event_window* parent_;
	position position_ = {0, 0};
	size size_ = {0, 0};

};

}
