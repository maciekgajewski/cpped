#pragma once
#include "ncurses_inc.hh"
#include "event_window.hh"

#include <vector>
#include <string>


namespace nct {



class event_dispatcher
{
public:

	using polling_function = std::function<bool()>;

	event_dispatcher() = default;
	event_dispatcher(const event_dispatcher&) = delete;
	void set_global_quit_key(const std::string& key_name) { quit_key_ = key_name; }

	// called every spin of the loop. Screen repainted if returns true
	void set_poll_function(const polling_function& pf) { poll_function_ = pf; }

	void exit();
	void run();

	event_window* get_active_window() const { return active_window_; }

private:

	// window interface
	void add_window(event_window* win);
	void remove_window(event_window* win);
	void set_active_window(event_window* win);

	void send_mouse_event(const MEVENT& ev);
	void send_special_key(int c, const char* key_name);
	void send_sequence(std::string& seq);

	void render_windows();
	void check_for_terminal_resize();

	window_set windows_;
	event_window* active_window_ = nullptr;
	bool run_ = true;
	std::string quit_key_;
	WINDOW* get_active_ncurses_window() const;

	polling_function poll_function_;
	nct::size screen_size_;

	friend class event_window;
};

}
