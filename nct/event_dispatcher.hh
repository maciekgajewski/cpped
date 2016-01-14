#pragma once
#include "ncurses_inc.hh"
#include "event_window.hh"

#include <vector>
#include <string>


namespace nct {



class window_manager
{
public:

	using polling_function = std::function<bool()>;

	window_manager() = default;
	window_manager(const window_manager&) = delete;

	void stdin_readable();

	event_window* get_active_window() const { return active_window_; }
	void render_windows();

private:

	// window interface
	void add_window(event_window* win);
	void remove_window(event_window* win);
	void set_active_window(event_window* win);

	void send_mouse_event(const MEVENT& ev);
	void send_special_key(int c, const char* key_name);
	void send_sequence(std::string& seq);

	void check_for_terminal_resize();

	window_set windows_;
	event_window* active_window_ = nullptr;
	WINDOW* get_active_ncurses_window() const;

	polling_function poll_function_;
	nct::size screen_size_;

	friend class event_window;
};

}
