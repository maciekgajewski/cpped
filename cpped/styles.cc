#include "styles.hh"

#include <ncurses.h>

namespace cpped
{

int cpped::color_palette::get_pair_for_colors(int bg, int fg)
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

		int pair = color_pairs.size();
		::init_pair(pair, fg, bg);

		color_pairs.insert(std::make_pair(key, pair));
		return pair;
	}
	else
	{
		return it->second;
	}
}

style_manager::style_manager()
{
	int bg = COLOR_BLACK; // default background

	attr_for_token[int(document::token_type::none)]			= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_WHITE));
	attr_for_token[int(document::token_type::keyword)]		= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_YELLOW));
	attr_for_token[int(document::token_type::literal)]		= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_CYAN));
	attr_for_token[int(document::token_type::preprocessor)]	= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_GREEN));
	attr_for_token[int(document::token_type::type)]			= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_RED));
	attr_for_token[int(document::token_type::comment)]		= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_BLUE));

	line_numbers = COLOR_PAIR(palette.get_pair_for_colors(COLOR_BLACK, COLOR_RED));
	visual_tab = COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_CYAN));
}

}
