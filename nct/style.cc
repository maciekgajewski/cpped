#include "style.hh"

#include "ncurses_env.hh"

namespace nct {

int style::to_attr() const
{
	return ncurses_env::get_current()->style_to_attr(*this);
}

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


}
