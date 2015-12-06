#pragma once

#include <vector>
#include <string>

#include <ncursesw/ncurses.h>

namespace cpped {

class event_window;

class event_dispatcher
{
public:

	event_dispatcher() = default;
	event_dispatcher(const event_dispatcher&) = delete;
	void set_global_quit_key(const std::string& key_name) { quit_key_ = key_name; }

	void exit();
	void run();

private:

	// window interface
	void add_window(event_window* win);
	void remove_window(event_window* win);
	void set_active_window(event_window* win);

	void send_mouse_event(const MEVENT& ev);
	void send_special_key(int c, const char* key_name);
	void send_sequence(std::string& seq);

	std::vector<event_window*> windows_;
	event_window* active_window_ = nullptr;
	bool run_ = true;
	std::string quit_key_;

	friend class event_window;
};

}
