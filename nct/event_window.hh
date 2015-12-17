#pragma once

#include "ncurses_window.hh"
#include "types.hh"

#include <boost/optional.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace nct {

class event_dispatcher;
class event_window;
class color_palette;

class window_set : public std::vector<event_window*>
{
public:
	void insert(event_window* win)
	{
		auto it = std::find(begin(), end(), win);
		if (it != end())
		{
			throw std::logic_error("window added twice");
		}
		push_back(win);
	}

	void remove(event_window* win)
	{
		auto it = std::find(begin(), end(), win);
		if (it == end())
		{
			throw std::logic_error("window removed twice");
		}
		erase(it);
	}
};

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
	bool is_active() const;
	virtual void on_activated() {}
	virtual void on_deactivated() {}

	void hide();
	void show();
	bool is_visible() const { return window_.is_initialized(); }

	// window parameters
	// position is always relative to the parent

	const size& get_size() const { return size_; }
	const position& get_position() const { return position_; }
	int get_z() const { return z_; }

	void move(const position& pos, const size& sz);
	void set_size(const size& sz) { move(position_, sz); }
	void set_position(const position& pos) { move(pos, size_); }

	// Returns reference to ncurses window, throws if hidden and ncurses window not created.
	// note: the reference may be invalidated by call to hide() or another method. Do not store!
	ncurses_window& get_ncurses_window();

	// called by the window system
	void do_show_cursor();
	void do_refresh();

protected:

	virtual void on_resized() {}

	void show_cursor(const position& pos) { requested_cursor_position_ = pos; }
	void hide_cursor() { requested_cursor_position_ = boost::none; }
	void refresh_window() { refresh_requested_ = true; if (parent_) parent_->refresh_window(); }

	// converts local to global (screen) coordinates
	position to_global(const position& pos);

	color_palette& get_palette();
	event_dispatcher& get_event_dispatcher() const { return event_dispatcher_; }

private:

	event_dispatcher& event_dispatcher_;
	boost::optional<ncurses_window> window_;
	int z_ = 0;
	event_window* parent_;
	position position_ = {0, 0};
	size size_ = {0, 0};
	window_set children_;
	boost::optional<position> requested_cursor_position_;
	bool refresh_requested_ = false; // TODO this is ignored now, remove if not used
};


}
