#include "fbuttons.hh"

#include <cstdio>

namespace nct {

static const unsigned NBUTTONS = 10;


fbuttons::fbuttons(event_dispatcher& ed, event_window* parent)
	: event_window(ed, parent)
{

}

void fbuttons::update()
{
	if (!is_visible()) return;
	ncurses_window& window = get_ncurses_window();

	style f_style = nct::style{COLOR_BLACK, COLOR_WHITE};
	style text_style = nct::style{COLOR_CYAN, COLOR_BLACK};

	char f_buf[8];

	window.style_fill_line(text_style, ' ', 0);
	for(unsigned i = 0; i < NBUTTONS; i++)
	{
		unsigned begin = std::floor(double(i) / NBUTTONS * window.get_width());
		unsigned end = std::floor(double(i+1) / NBUTTONS * window.get_width());

		int fl = std::snprintf(f_buf, 8, "F%d", i+1);
		window.move_cursor(0, begin);
		window.style_print(f_style, f_buf, fl);

		if (end > (begin+fl))
		{
			// TODO print text here
			window.style_print(text_style, get_text(i));
		}
	}
}

std::string fbuttons::get_text(unsigned idx) const
{
	// TODO
	return {};
}

}
