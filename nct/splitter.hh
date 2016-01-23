#pragma once

#include "event_window.hh"

#include <boost/signals2.hpp>

#include <array>

namespace nct {

// Splitter is a window split in two, containing two other windows, managing they positions
template<typename SplitterDirection>
class splitter : public nct::event_window
{
public:

	splitter(window_manager& wm, event_window* parent);

	void set_fixed(unsigned idx, nct::event_window* win, unsigned requested_size);
	void set_stretching(unsigned idx, nct::event_window* win);

private:

	void on_resized() override;
	void render(nct::ncurses_window& surface) override;

	void recalculate_sizes();
	unsigned get_availabale_size() const;
	void apply_size(nct::event_window* win, unsigned size_before, unsigned size);

	enum class entry_type { fixed, stretching, unset };

	struct entry
	{
		nct::event_window* window_ = nullptr;
		entry_type type_ = entry_type::unset;
		unsigned requested_size_ = 0;
		unsigned size_;
		boost::signals2::scoped_connection title_chaned_connection_;
	};

	std::array<entry, 2> entries_;

};

namespace _splitter_directions
{
	struct horizontal {};
	struct vertical {};
}

using horizontal_splitter = splitter<_splitter_directions::horizontal>;
using vertical_splitter = splitter<_splitter_directions::vertical>;

}
