#include "ncurses_inc.hh"

#include "event_window.hh"

#include "ncurses_env.hh"
#include "event_dispatcher.hh"

namespace nct {

event_window::event_window(event_dispatcher& ed, event_window* parent)
	: event_dispatcher_(ed), parent_(parent)
{
	if (parent)
		parent->children_.insert(this);
	else
		event_dispatcher_.add_window(this);
}

event_window::~event_window()
{
	if (parent_)
		parent_->children_.remove(this);
	else
		event_dispatcher_.remove_window(this);

	for(event_window* child: children_)
	{
		child->parent_ = nullptr;
		event_dispatcher_.add_window(child);
	}
}

void event_window::set_active()
{
	event_dispatcher_.set_active_window(this);
	on_activated();
}

bool event_window::is_active() const
{
	return event_dispatcher_.get_active_window() == this;
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
		position global_pos = to_global({0 ,0});
		window_.emplace(size_.h, size_.w, global_pos.y, global_pos.x);
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
				window_->move(pos.y, pos.x);
			}
		}
		position_ = pos;
		size_ = sz;
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

	if (window_)
	{
//		if (refresh_requested_)
//		{
			window_->redraw();
			window_->no_out_refresh();

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

color_palette& event_window::get_palette()
{
	return event_dispatcher_.get_palette();
}

}
