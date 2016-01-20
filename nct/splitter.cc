#include "splitter.hh"

namespace nct {

template<typename SplitterDirection>
splitter<SplitterDirection>::splitter(window_manager& wm, event_window* parent)
	: nct::event_window(wm, parent)
{
}

template<typename SplitterDirection>
void splitter<SplitterDirection>::on_resized()
{
	if (entires_[0].type == entry_type::unset && entries_[1].type == entry_type::unset)
	{
		return; // do nothing
	}

	// split size
	unsigned size = SplitterDirection::get_size(get_size());
}

}
