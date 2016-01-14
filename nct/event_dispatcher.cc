#include "ncurses_inc.hh"

#include "event_dispatcher.hh"

#include "event_window.hh"
#include "ncurses_env.hh"

#include <algorithm>
#include <stdexcept>

namespace nct {

void window_manager::stdin_readable()
{
	WINDOW* active_window = get_active_ncurses_window();
	::wtimeout(active_window, 0); // enter non-blocking mode

	std::string input_buffer;
	MEVENT mouse_event;

	for(int c = ::wgetch(active_window); c != ERR; c = ::wgetch(active_window))
	{
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
				send_special_key(c, key_name);
			}
		}
	}
	check_for_terminal_resize();
	send_sequence(input_buffer);
}

void window_manager::add_window(event_window* win)
{
	windows_.insert(win);
}

void window_manager::remove_window(event_window* win)
{
	windows_.remove(win);
	if (active_window_ == win)
		active_window_ = nullptr;
}

void window_manager::set_active_window(event_window* win)
{
	if (active_window_ != win)
	{
		if (active_window_)
			active_window_->on_deactivated();
		active_window_ = win;
	}
}

void window_manager::send_mouse_event(const MEVENT& ev)
{
	// TODO
	// look for the mouse under the cursor
}

void window_manager::send_special_key(int c, const char* key_name)
{
	event_window* win = active_window_;

	while(win)
	{
		if (win->on_special_key(c, key_name))
		{
			render_windows();
			break;
		}
		else
		{
			// propagate event to parent window
			win = win->get_parent();
		}
	}
}

void window_manager::send_sequence(std::string& seq)
{
	event_window* win = active_window_;
	unsigned remaining_chars = seq.length();

	if (win && remaining_chars > 0)
	{
		while(win && remaining_chars > 0)
		{
			unsigned consumed = win->on_sequence(seq);
			if (consumed > remaining_chars)
				throw std::logic_error("on_sequence returned more than sequnece length");

			seq.erase(0, consumed);
			remaining_chars = seq.length();
			win = win->get_parent();
		}
		render_windows();
	}
	seq.clear();
}

void window_manager::render_windows()
{
	std::sort(windows_.begin(), windows_.end(),
		[](const event_window* a, const event_window* b) { return a->get_z() < b->get_z(); });

	for(event_window* w : windows_)
	{
		w->do_refresh();
	}
	if (active_window_)
		active_window_->do_show_cursor();
	::doupdate();

}

void window_manager::check_for_terminal_resize()
{
	nct::size sz = nct::ncurses_env::get_current()->get_stdscr().get_size();
	if (sz != screen_size_)
	{
		screen_size_ = sz;
		render_windows();
	}
}

WINDOW* window_manager::get_active_ncurses_window() const
{
	if (active_window_ && active_window_->is_visible())
	{
		return active_window_->get_ncurses_window().get_window();
	}
	else
	{
		return ::stdscr;
	}
}

}
