#include "styles.hh"

#include <ncursesw/ncurses.h>

namespace cpped
{

style_manager::style_manager()
{
	int bg = COLOR_BLACK; // default background

	style_for_token[int(document::token_type::none)]			= nct::style{bg, COLOR_WHITE};
	style_for_token[int(document::token_type::keyword)]		= nct::style{bg, COLOR_YELLOW};
	style_for_token[int(document::token_type::literal)]		= nct::style{bg, COLOR_CYAN};
	style_for_token[int(document::token_type::preprocessor)]	= nct::style{bg, COLOR_MAGENTA};
	style_for_token[int(document::token_type::type)]			= nct::style{bg, COLOR_GREEN};
	style_for_token[int(document::token_type::comment)]		= nct::style{bg, COLOR_BLUE};

	plain = nct::style{bg, COLOR_WHITE};
	line_numbers = nct::style{COLOR_BLACK, COLOR_RED};
	visual_tab = nct::style{bg, COLOR_BLUE};
	status = nct::style{bg, COLOR_WHITE};
}

}
