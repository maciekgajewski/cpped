#include <ncurses.h>

#include "editor.hh"
#include "ncurses_window.hh"

#include "document_lib/document.hh"

#include <string>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cassert>

namespace cpped {

editor::editor(ncurses_window& win, document::document& d)
	: window(win)
	, doc(&d)
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

	//doc->render(window, first_line, first_column, get_workspace_height(), get_workspace_width());
	refresh_cursor();;
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
		int new_line_len = doc->line_length(cursor_doc_y);
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
		int new_line_len = doc->line_length(cursor_doc_y);
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
	int wx = documet_to_workspace_x(cursor_doc_x) + doc->left_bar_width();
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

int editor::get_workspace_width() const
{
	assert(doc);
	return window.get_width() - doc->left_bar_width();
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
