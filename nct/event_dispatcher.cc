#include "event_dispatcher.hh"

#include "event_window.hh"

#include <ncursesw/ncurses.h>

#include <algorithm>
#include <stdexcept>

namespace nct {

void event_dispatcher::exit()
{
	run_ = false;
}

void event_dispatcher::run()
{
	run_ = true;
	std::string input_buffer;
	MEVENT mouse_event;

	::timeout(0);

	while(run_)
	{

		int c = ::getch();

		if (c == ERR)
		{
			send_sequence(input_buffer);
			// no input, back off, wait for more
			::timeout(10); // pool every 10ms

			while(true)
			{
				c = ::getch();

				// TODO poll for events here

				if (c != ERR)
				{
					::timeout(0);
					break;
				}
			}
		}

		if(::getmouse(&mouse_event) == OK)
		{
			send_sequence(input_buffer);
			send_mouse_event(mouse_event);
		}
		else
		{
			// distinguish between regular/special
			if ((c > 31 && c < 256) || c == '\n' || c == '\t')
			{
				input_buffer.push_back(c);
			}
			else
			{
				send_sequence(input_buffer);
				// special char
				const char* key_name = ::keyname(c);
				if (key_name == quit_key_)
				{
					return;
				}
				send_special_key(c, key_name);
			}
		}
	}
}

void event_dispatcher::add_window(event_window* win)
{
	auto it = std::find(windows_.begin(), windows_.end(), win);
	if (it != windows_.end())
	{
		throw std::logic_error("window added twice");
	}
	windows_.push_back(win);
}

void event_dispatcher::remove_window(event_window* win)
{
	auto it = std::find(windows_.begin(), windows_.end(), win);
	if (it == windows_.end())
	{
		throw std::logic_error("window removed twice");
	}
	windows_.erase(it);
	if (active_window_ == win)
		active_window_ = nullptr;
}

void event_dispatcher::set_active_window(event_window* win)
{
	active_window_ = win;
}

void event_dispatcher::send_mouse_event(const MEVENT& ev)
{
	// TODO
	// look for the mouse under the cursor
}

void event_dispatcher::send_special_key(int c, const char* key_name)
{
	event_window* win = active_window_;

	while(win && !win->on_special_key(c, key_name))
	{
		win = win->get_parent();
	}
}

void event_dispatcher::send_sequence(std::string& seq)
{
	event_window* win = active_window_;
	unsigned remaining_chars = seq.length();

	while(win && remaining_chars > 0)
	{
		unsigned consumed = win->on_sequence(seq);
		if (consumed > remaining_chars)
			throw std::logic_error("on_sequence returned more than sequnece length");

		seq.erase(0, seq.length() - consumed);
		remaining_chars = seq.length();
		win = win->get_parent();
	}
	seq.clear();
}

}
