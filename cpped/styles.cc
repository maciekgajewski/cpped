#include "styles.hh"

#include <ncursesw/ncurses.h>

namespace cpped
{

style_manager::style_manager(nct::color_palette& pal)
	 : palette(pal)
{
	int bg = COLOR_BLACK; // default background

	attr_for_token[int(document::token_type::none)]			= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_WHITE));
	attr_for_token[int(document::token_type::keyword)]		= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_YELLOW));
	attr_for_token[int(document::token_type::literal)]		= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_CYAN));
	attr_for_token[int(document::token_type::preprocessor)]	= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_MAGENTA));
	attr_for_token[int(document::token_type::type)]			= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_GREEN));
	attr_for_token[int(document::token_type::comment)]		= COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_BLUE));

	line_numbers = COLOR_PAIR(palette.get_pair_for_colors(COLOR_BLACK, COLOR_RED));
	visual_tab = COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_CYAN));
	status = COLOR_PAIR(palette.get_pair_for_colors(bg, COLOR_WHITE));
}

}
