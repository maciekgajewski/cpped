#pragma once
#include "ncurses_inc.hh"
#include "event_window.hh"

#include <boost/container/flat_map.hpp>

#include <vector>
#include <string>


namespace nct {

class color_palette
{
public:

	int get_pair_for_colors(int bg, int fg);

private:
	boost::container::flat_map<std::pair<int, int>, int> color_pairs;
};


class event_dispatcher
{
public:

	event_dispatcher() = default;
	event_dispatcher(const event_dispatcher&) = delete;
	void set_global_quit_key(const std::string& key_name) { quit_key_ = key_name; }

	// called every spin of the loop
	void set_poll_function(const std::function<void()>& pf) { poll_function_ = pf; }

	void exit();
	void run();

	color_palette& get_palette() { return palette_; }
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

	window_set windows_;
	event_window* active_window_ = nullptr;
	bool run_ = true;
	std::string quit_key_;
	WINDOW* get_active_ncurses_window() const;

	color_palette palette_;
	std::function<void()> poll_function_;

	friend class event_window;
};

}
