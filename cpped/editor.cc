#include <ncurses.h>

#include "editor.hh"
#include "ncurses_window.hh"
#include "styles.hh"

#include "document_lib/document.hh"

#include <string>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cassert>
#include <cstdio>

namespace cpped {

editor::editor(ncurses_window& win, document::document& d, style_manager& sm)
	: window(win)
	, doc(&d)
	, styles(sm)
{
}

void editor::on_key(int key)
{

	// hard-coded actions
	switch(key)
	{
		case KEY_UP:
			cursor_up();
			break;
		case KEY_DOWN:
			cursor_down();
			break;
		case KEY_LEFT:
			cursor_left();
			break;
		case KEY_RIGHT:
			cursor_right();
			break;

		default:
		{
			std::ostringstream ss;
			ss << "key=" << key << ", name=" << ::keyname(key);
			window.clear();
			window.print(ss.str().c_str());
		}
	}
}

void editor::on_mouse(const MEVENT& event)
{
	std::ostringstream ss;
	ss << "mouse x=" << event.x << ", y=" << event.y << ", buttons=" << event.bstate;
	window.clear();
	window.print(ss.str().c_str());
}

void editor::render()
{
	window.clear();

	int line_count_digits = 8;
	if (doc->get_line_count() < 10)
		line_count_digits = 1;
	else if (doc->get_line_count() < 100)
		line_count_digits = 2;
	else if (doc->get_line_count() < 1000)
		line_count_digits = 3;
	else if (doc->get_line_count() < 10000)
		line_count_digits = 4;

	left_margin_width = line_count_digits + 2;
	char fmt[32];
	std::snprintf(fmt, 32, " %%%dd ", line_count_digits);
	char lineno[32];

	// iterate over lines
	int line_no = 0;
	doc->for_lines(first_line, window.get_height(), [&](const document::document_line& line)
	{
		window.move(line_no, 0);

		// print line number
		std::snprintf(lineno, 32, fmt, first_line+line_no++);
		window.attr_print(styles.line_numbers, lineno, left_margin_width);

		// print line
		unsigned column = 0;
		line.for_each_token([&](const document::line_token& token)
		{
			unsigned begin = std::max(token.begin, first_column);
			unsigned end = std::min(get_workspace_width(), token.end-first_column);

			int attr = styles.get_attr_for_token(token.type);

			column = render_text(attr, column, line.get_data() + begin, line.get_data() + end);
		});
	});

	refresh_cursor();
}

void editor::set_document(document::document& d)
{
	doc = &d;
	cursor_doc_x = 0;
	cursor_doc_y = 0;
	desired_cursor_x = 0;
	first_line = 0;
	first_column = 0;
	render();
}

void editor::cursor_up()
{
	if (cursor_doc_y > 0)
	{
		cursor_doc_y--;
		unsigned new_line_len = doc->line_length(cursor_doc_y);
		if (cursor_doc_x > new_line_len)
		{
			cursor_doc_x = new_line_len;
		}
		else if (desired_cursor_x > cursor_doc_x)
		{
			cursor_doc_x = std::min(new_line_len, desired_cursor_x);
		}

		if (documet_to_workspace_y(cursor_doc_y) ==-1)
		{
			assert(first_line > 0);
			first_line--;
			render();
		}
		else
		{
			refresh_cursor();
		}
	}
}

void editor::cursor_down()
{
	assert(doc);

	if (cursor_doc_y < doc->get_line_count())
	{
		cursor_doc_y++;
		unsigned new_line_len = doc->line_length(cursor_doc_y);
		if (cursor_doc_x > new_line_len)
		{
			cursor_doc_x = new_line_len;
		}
		else if (desired_cursor_x > cursor_doc_x)
		{
			cursor_doc_x = std::min(new_line_len, desired_cursor_x);
		}

		if (documet_to_workspace_y(cursor_doc_y) == get_workspace_height())
		{
			// scroll one line down
			first_line ++;
			render();
		}
		else
		{
			// just move cursor
			refresh_cursor();
		}
	}
}

void editor::cursor_left()
{
	if (cursor_doc_x > 0)
	{
		cursor_doc_x--;
		desired_cursor_x = cursor_doc_x;

		if (documet_to_workspace_x(cursor_doc_x) == -1)
		{
			// scroll left
			assert(first_column > 0);
			first_column--;
			render();
		}
		else
		{
			refresh_cursor();
		}
	}
}

void editor::cursor_right()
{
	assert(doc); // TODO is this needed?

	int ll = doc->line_length(cursor_doc_y);
	if (cursor_doc_x < ll)
	{
		cursor_doc_x++;
		desired_cursor_x = cursor_doc_x;

		if(documet_to_workspace_x(cursor_doc_x) == get_workspace_width())
		{
			// scroll right
			first_column++;
			render();
		}
		else
		{
			// just move cursor
			refresh_cursor();
		}
	}
}

void editor::scroll_down()
{
	// TODO
}

void editor::scroll_up()
{
	// TODO
}

void editor::scroll_left()
{
	// TODO
}

void editor::scroll_right()
{
	// TODO
}

void editor::refresh_cursor()
{
	int wx = documet_to_workspace_x(cursor_doc_x) + left_margin_width;
	int wy = documet_to_workspace_y(cursor_doc_y);
	if (wx >= 0 && wy >= 0 && wx < get_workspace_width() && wy < get_workspace_height())
	{
		::curs_set(1);
		window.move(wy, wx);
	}
	else
	{
		::curs_set(0); // hide cursor
	}

}

unsigned editor::render_text(attr_t attr, unsigned phys_column, const char* begin, const char* end)
{
	window.set_attr_on(attr);
	unsigned last_column = get_workspace_width() + first_column;

	while(begin != end && phys_column != last_column)
	{
		if(*begin == '\t')
		{
			// render tab
			unsigned w = phys_column+tab_width - (phys_column+tab_width%4);
			for(unsigned c = 0; c < w && phys_column != last_column; c++, phys_column++)
			{
				if (phys_column >= first_column)
				{
					if (w == tab_width && c == 0) // first char of full tab
						put_visual_tab();
					else
						window.put_char(' ');
				}
			}
		}
		else
		{
			if (phys_column >= first_column)
				window.put_char(*begin);
			phys_column++;
		}
		begin++;
	}

	window.set_attr_off(attr);

	return phys_column;
}

void editor::put_visual_tab()
{
	if (visualise_tabs)
	{
		window.set_attr_on(styles.visual_tab);
		window.put_char('|'); // TODO maybe use some cool unicode char?
		window.set_attr_off(styles.visual_tab);
	}
	else
	{
		window.put_char(' ');
	}
}

unsigned editor::get_workspace_width() const
{
	assert(doc);
	if (window.get_width() < left_margin_width)
		return 0;
	else
		return window.get_width() - left_margin_width;
}

int editor::get_workspace_height() const
{
	return window.get_height();
}

int editor::documet_to_workspace_x(int docx) const
{
	return docx - first_column;
}

int editor::documet_to_workspace_y(int docy) const
{
	return docy - first_line;
}

int editor::workspace_to_document_x(int wx) const
{
	return wx + first_column;
}

int editor::workspace_to_document_y(int wy) const
{
	return wy + first_line;
}



}
