#include "splitter.hh"

namespace nct {

namespace  _splitter_directions {

void horizontal::apply_size(nct::event_window* win, const nct::size& splitter_size, unsigned size_before, unsigned size)
{
	nct::position pos{1, int(size_before)};
	nct::size sz { int(splitter_size.h - 2), int(size)};
	win->move(pos, sz);
}

void vertical::apply_size(nct::event_window* win, const nct::size& splitter_size, unsigned size_before, unsigned size)
{
	nct::position pos{int(size_before), 1};
	nct::size sz { int(size), int(splitter_size.w - 2)};
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

	unsigned size0 = 0;
	unsigned size1 = 0;
	if(entries_[0].type_ == entry_type::unset)
	{
		size0 = 0;
		size1 = size-2;
	}
	else if (entries_[1].type_ == entry_type::unset)
	{
		size0 = size-2;
		size1 = 0;
	}
	else
	{
		if (entries_[0].type_ == entry_type::fixed && entries_[1].type_ == entry_type::stretching)
		{
			size0 = size_of_fixed(size, entries_[0].requested_size_) - 1;
			size1 = size - size0 - 2;
		}
		else if (entries_[0].type_ == entry_type::fixed && entries_[1].type_ == entry_type::stretching)
		{
			size1 = size_of_fixed(size, entries_[0].requested_size_) - 1;
			size0 = size - size1 - 2;
		}
		else
		{
			size0 = size/2 - 1;
			size1 = size - size0 - 2;
		}
	}

	if(entries_[0].window_)
	{
		SplitterDirection::apply_size(entries_[0].window_, get_size(), 1, size0);
	}
	if(entries_[1].window_)
	{
		SplitterDirection::apply_size(entries_[1].window_, get_size(), size0 + 2, size1);
	}

}


template class splitter<_splitter_directions::horizontal>;
template class splitter<_splitter_directions::vertical>;

}
