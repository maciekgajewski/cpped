#pragma once

#include "ncurses_window.hh"
#include "types.hh"

#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace nct {

class window_manager;
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
	event_window(window_manager& wm, event_window* parent);
	virtual ~event_window();

	// signals
	boost::signals2::signal<void(const std::string&)> title_changed_signal;

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
	virtual void render(nct::ncurses_window& surface) {}

	event_window* get_parent() const { return parent_; }
	void set_parent(event_window* parent);

	// window state manipulation

	void set_active();
	bool is_active() const;
	virtual void on_activated() {}
	virtual void on_deactivated() {}

	void hide();
	void show();
	bool is_visible() const { return visible_; }

	// window parameters
	// position is always relative to the parent

	const size& get_size() const { return size_; }
	const position& get_position() const { return position_; }
	int get_z() const { return z_; }
	position get_global_position() const { return to_global(position_); }

	void move(const position& pos, const size& sz);
	void set_size(const size& sz) { move(position_, sz); }
	void set_position(const position& pos) { move(pos, size_); }
	void set_fullscreen(bool fs);

	void set_title(const std::string& title);
	const std::string& get_title() const { return title_; }

protected:

	virtual void on_resized() {}

	void show_cursor(const position& pos) { requested_cursor_position_ = pos; }
	void hide_cursor() { requested_cursor_position_ = boost::none; }
	// Request window redraw during the next cycle. 'render()' wil be called when the redraw is processed
	void request_redraw() { redraw_requested_ = true; if (parent_) parent_->request_redraw(); }
	// Performs redraw here and now
	void redraw_now();

	// converts local to global (screen) coordinates
	position to_global(const position& pos) const;

	window_manager& get_window_manager() const { return window_manager_; }

private: // called by wm

	void do_show_cursor();
	void do_render();
	ncurses_window& get_ncurses_window();
	bool is_redraw_requested() const { return redraw_requested_; }

	friend class window_manager;

private:

	void destroy_surface();
	void create_surface();
	void parent_moved();

	window_manager& window_manager_;
	boost::optional<ncurses_window> window_;
	int z_ = 0;
	event_window* parent_;
	position position_ = {0, 0};
	size size_ = {0, 0};
	window_set children_;
	boost::optional<position> requested_cursor_position_;
	bool redraw_requested_ = false;
	bool fullscreen_ = false;
	bool visible_ = true;
	std::string title_;
};


}
