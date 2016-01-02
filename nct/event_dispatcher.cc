#include "ncurses_inc.hh"

#include "event_dispatcher.hh"

#include "event_window.hh"
#include "ncurses_env.hh"

#include <algorithm>
#include <stdexcept>

namespace nct {

int color_palette::get_pair_for_colors(int bg, int fg)
{
	auto key = std::make_pair(bg, fg);
	auto it = color_pairs.find(key);
	if (it == color_pairs.end())
	{
		// new pair is needed
		if (color_pairs.size() == COLOR_PAIRS)
		{
			throw std::runtime_error("All color pairs used");
		}

		int pair = color_pairs.size() + 1; // pair can't be 0
		::init_pair(pair, fg, bg);

		color_pairs.insert(std::make_pair(key, pair));
		return pair;
	}
	else
	{
		return it->second;
	}
}


void event_dispatcher::exit()
{
	run_ = false;
}

void event_dispatcher::run()
{
	run_ = true;
	screen_size_ = nct::ncurses_env::get_current()->get_stdscr().get_size();
	std::string input_buffer;
	MEVENT mouse_event;

	render_windows(); // draw any changes that has been shcheduled before the call

	while(run_)
	{
		WINDOW* active_window = get_active_ncurses_window();
		::wtimeout(active_window, 0); // enter non-blocking mode
		int c = ::wgetch(active_window);

		if (c == ERR)
		{
			send_sequence(input_buffer);

			// Finished processing keys, poll before going down
			if (poll_function_ && poll_function_())
			{
				render_windows();
			}

			check_for_terminal_resize();

			// no input, back off, wait for more
			::wtimeout(active_window, 10); // pool every 10ms

			while(true)
			{
				c = ::wgetch(get_active_ncurses_window());

				if (c != ERR)
				{
					::wtimeout(active_window, 0);
					break;
				}
				else
				{
					if (poll_function_ && poll_function_())
					{
						render_windows();
					}
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
	windows_.insert(win);
}

void event_dispatcher::remove_window(event_window* win)
{
	windows_.remove(win);
	if (active_window_ == win)
		active_window_ = nullptr;
}

void event_dispatcher::set_active_window(event_window* win)
{
	if (active_window_ != win)
	{
		if (active_window_)
			active_window_->on_deactivated();
		active_window_ = win;
	}
}

void event_dispatcher::send_mouse_event(const MEVENT& ev)
{
	// TODO
	// look for the mouse under the cursor
}

void event_dispatcher::send_special_key(int c, const char* key_name)
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

void event_dispatcher::send_sequence(std::string& seq)
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

void event_dispatcher::render_windows()
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

void event_dispatcher::check_for_terminal_resize()
{
	nct::size sz = nct::ncurses_env::get_current()->get_stdscr().get_size();
	if (sz != screen_size_)
	{
		screen_size_ = sz;
		render_windows();
	}
}

WINDOW* event_dispatcher::get_active_ncurses_window() const
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
