#include "splitter.hh"

namespace nct {

namespace  _splitter_directions {

void horizontal::apply_size(nct::event_window* win, const nct::size& splitter_size, unsigned size_before, unsigned size)
{
	nct::position pos{0, int(size_before)};
	nct::size sz { int(splitter_size.h), int(size)};
	win->move(pos, sz);
}

void vertical::apply_size(nct::event_window* win, const nct::size& splitter_size, unsigned size_before, unsigned size)
{
	nct::position pos{int(size_before), 0};
	nct::size sz { int(size), int(splitter_size.w)};
	win->move(pos, sz);
}

} // _splitter_directions

template<typename SplitterDirection>
splitter<SplitterDirection>::splitter(window_manager& wm, event_window* parent)
	: nct::event_window(wm, parent)
{
}

template<typename SplitterDirection>
void splitter<SplitterDirection>::set_fixed(unsigned idx, event_window* win, unsigned requested_size)
{
	entries_[idx].window_ = win;
	entries_[idx].type_ = entry_type::fixed;
	entries_[idx].requested_size_ = requested_size;

	win->set_parent(this);
	recalculate_sizes();
	request_redraw();
}

template<typename SplitterDirection>
void splitter<SplitterDirection>::set_stretching(unsigned idx, event_window* win)
{
	entries_[idx].window_ = win;
	entries_[idx].type_ = entry_type::stretching;

	win->set_parent(this);
	recalculate_sizes();
	request_redraw();
}

static unsigned size_of_fixed(unsigned total, unsigned requested)
{
	if (total > 80)
		return std::min(requested, total);
	else
		return std::min<unsigned>(total, requested*total/80.0);
}

template<typename SplitterDirection>
void splitter<SplitterDirection>::on_resized()
{
	recalculate_sizes();
}

template<>
void splitter<_splitter_directions::horizontal>::render(nct::ncurses_window& surface)
{
	if (entries_[0].type_ == entry_type::unset || entries_[0].type_ == entry_type::unset)
		return; // invalid state

	// split line
	surface.vertical_line(0, entries_[0].size_, ACS_VLINE, surface.get_height());
	surface.move_cursor(0, entries_[0].size_);
	surface.put_char(ACS_TTEE);

}

template<>
void splitter<_splitter_directions::vertical>::render(ncurses_window& surface)
{
	// TODO
}

template<typename SplitterDirection>
void splitter<SplitterDirection>::recalculate_sizes()
{
	if (entries_[0].type_ == entry_type::unset && entries_[1].type_ == entry_type::unset)
	{
		return; // do nothing
	}

	// split size
	unsigned size = SplitterDirection::get_size(get_size());

	if (size == 0) // invalid state
		return;

	entries_[0].size_ = 0;
	entries_[1].size_ = 0;
	if(entries_[0].type_ == entry_type::unset)
	{
		entries_[0].size_ = 0;
		entries_[1].size_ = size-1;
	}
	else if (entries_[1].type_ == entry_type::unset)
	{
		entries_[0].size_ = size-1;
		entries_[1].size_ = 0;
	}
	else
	{
		if (entries_[0].type_ == entry_type::fixed && entries_[1].type_ == entry_type::stretching)
		{
			entries_[0].size_ = size_of_fixed(size, entries_[0].requested_size_);
			entries_[1].size_ = size - entries_[0].size_ - 1;
		}
		else if (entries_[0].type_ == entry_type::fixed && entries_[1].type_ == entry_type::stretching)
		{
			entries_[1].size_ = size_of_fixed(size, entries_[0].requested_size_);
			entries_[0].size_ = size - entries_[1].size_ - 1;
		}
		else
		{
			entries_[0].size_ = size/2 - 1;
			entries_[1].size_ = size - entries_[0].size_;
		}
	}

	if(entries_[0].window_)
	{
		SplitterDirection::apply_size(entries_[0].window_, get_size(), 0, entries_[0].size_);
	}
	if(entries_[1].window_)
	{
		SplitterDirection::apply_size(entries_[1].window_, get_size(), entries_[0].size_ + 1, entries_[1].size_);
	}
}


template class splitter<_splitter_directions::horizontal>;
template class splitter<_splitter_directions::vertical>;

}
