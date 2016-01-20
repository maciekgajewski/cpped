#pragma once

#include "event_window.hh"

#include <array>

namespace nct {

// Splitter is a window split in two, containing two other windows, managing they positions
template<typename SplitterDirection>
class splitter : public nct::event_window
{
public:

	splitter(window_manager& wm, event_window* parent);

private:

	void on_resized() override;

	enum class entry_type { fixed, stretching, unset };

	struct entry
	{
		nct::event_window* window_ = nullptr;
		entry_type type_ = entry_type::unset;
		unsigned size_ = 0;
	};

	std::array<entry, 2> entires_;

};

namespace _splitter_directions
{

struct horizontal
{
	static unsigned get_size(const size& sz) { return sz.w; }
};

struct vertical
{
	static unsigned get_size(const size& sz) { return sz.h; }
};

}

using horizontal_splitter = splitter<_splitter_directions::horizontal>;
using vertical_splitter = splitter<_splitter_directions::vertical>;

}
