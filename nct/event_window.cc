#include "event_window.hh"

#include "event_dispatcher.hh"

namespace nct {

event_window::event_window(event_dispatcher& ed, event_window* parent)
	: event_dispatcher_(ed), parent_(parent)
{
	event_dispatcher_.add_window(this);
}

event_window::~event_window()
{
	event_dispatcher_.remove_window(this);
}

void event_window::set_active()
{
	event_dispatcher_.set_active_window(this);
}

void event_window::hide()
{
	if (window_)
	{
		window_.reset();
		on_hidden();
	}
}

void event_window::show()
{
	if (!window_)
	{
		position global_pos = to_global(position_);
		window_.emplace(size_.h, size_.w, global_pos.y, global_pos.x);
		on_shown();
	}
}

void event_window::move(const position& pos, const size& sz)
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
