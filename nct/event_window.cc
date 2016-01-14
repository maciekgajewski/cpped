#include "ncurses_inc.hh"

#include "event_window.hh"

#include "ncurses_env.hh"
#include "window_manager.hh"

namespace nct {

event_window::event_window(window_manager& wm, event_window* parent)
	: window_manager_(wm), parent_(parent)
{
	if (parent)
		parent->children_.insert(this);
	else
		window_manager_.add_window(this);
}

event_window::~event_window()
{
	if (parent_)
		parent_->children_.remove(this);
	else
		window_manager_.remove_window(this);

	for(event_window* child: children_)
	{
		child->parent_ = nullptr;
		window_manager_.add_window(child);
	}
}

void event_window::set_active()
{
	window_manager_.set_active_window(this);
	on_activated();
}

bool event_window::is_active() const
{
	return window_manager_.get_active_window() == this;
}

void event_window::hide()
{
	if (window_)
	{
		window_.reset();
		on_hidden();
		for(event_window* child : children_)
			child->hide();
	}
}

void event_window::show()
{
	if (!window_)
	{
		if (size_.h > 0 && size_.w > 0)
		{
			position global_pos = to_global({0 ,0});
			window_.emplace(size_.h, size_.w, global_pos.y, global_pos.x);
		}
		on_shown();
		for(event_window* child : children_)
			child->show();
	}
}

void event_window::move(const position& pos, const size& sz)
{
	if (pos != position_ || sz != size_)
	{
		if (window_)
		{
			if (sz != size_)
			{
				window_->resize(sz.h, sz.w);
			}
			if (pos != position_)
			{
				position_ = pos;
				position global_pos = to_global({0 ,0});
				window_->move(global_pos.y, global_pos.x);
			}
		}
		size_ = sz;
		position_ = pos;
		on_resized();
	}
}

ncurses_window&event_window::get_ncurses_window()
{
	if (window_)
	{
		return *window_;
	}
	else
	{
		throw std::logic_error("Attempt to access hidden window");
	}
}

void event_window::do_show_cursor()
{
	if (window_ && requested_cursor_position_)
	{
		::curs_set(1);
		window_->move_cursor(*requested_cursor_position_);
	}
	else
	{
		::curs_set(0);
	}
}

void event_window::do_refresh()
{
	if (fullscreen_)
	{
		set_size(nct::ncurses_env::get_current()->get_stdscr().get_size());
	}

	if (window_ || size_ == nct::size{0, 0})
	{
//		if (refresh_requested_)
//		{
			if (window_)
			{
				window_->redraw();
				window_->no_out_refresh();
			}

			for(event_window* child : children_)
				child->do_refresh();

			refresh_requested_ = false;
//		}
	}

}

void event_window::set_fullscreen(bool fs)
{
	if (fs != fullscreen_)
	{
		fullscreen_ = fs;
		if (fullscreen_)
		{
			set_size(nct::ncurses_env::get_current()->get_stdscr().get_size());
		}
	}
}

position event_window::to_global(const position& pos)
{
	if (parent_)
	{
		return parent_->to_global(position_ + pos);
	}
	else
	{
		return position_ + pos;
	}
}

}
