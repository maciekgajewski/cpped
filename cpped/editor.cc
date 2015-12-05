#include "editor.hh"

#include "editor_window.hh"

#include "document_lib/document.hh"


namespace cpped {

editor::editor(editor_window& win, document::document& d)
	: doc_(d),
	window_(win)
{
	request_full_render();
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
			if ((key >=32 && key < 256) || key == '\n' || key == '\t')
			{
				insert_at_cursor(char(key));
			}
		}
	}
}

void editor::on_mouse(const MEVENT& event)
{
	// ?
}

void editor::cursor_up()
{
	if (cursor_y_ > 0)
	{
		cursor_y_--;
		unsigned new_line_len = doc_.line_length(cursor_y_);
		if (cursor_x_ > new_line_len)
		{
			cursor_x_ = new_line_len;
		}

		adjust_cursor_column_to_desired(new_line_len);

		if (documet_to_workspace_y(cursor_y_) ==-1)
		{
			assert(first_line_ > 0);
			first_line_--;

			request_full_render();
		}
		else
		{
			request_cursor_update();
		}
	}
}

void editor::adjust_cursor_column_to_desired(unsigned new_line_len)
{
	unsigned current_column = document_x_to_column(cursor_y_, cursor_x_);
	while(current_column < desired_cursor_column_ && cursor_x_ < new_line_len)
	{
		cursor_x_++;
		current_column = document_x_to_column(cursor_y_, cursor_x_);
	}
	while(current_column > desired_cursor_column_ + tab_width_ && cursor_x_ > 0)
	{
		cursor_x_--;
		current_column = document_x_to_column(cursor_y_, cursor_x_);
	}
}

void editor::request_full_render()
{
	window_.render(first_column_, first_line_, tab_width_);
	request_cursor_update();
}

void editor::request_cursor_update()
{
	int column = document_x_to_column(cursor_y_, cursor_x_);
	int cx = column - int(first_column_);
	int cy = int(cursor_y_ )- int(first_line_);

	window_.refresh_cursor(cx, cy);
	window_.update_status_line(cursor_y_, cursor_x_, column);
}

void editor::cursor_down()
{
	if (cursor_y_ < doc_.get_line_count())
	{
		cursor_y_++;
		unsigned new_line_len = doc_.line_length(cursor_y_);
		if (cursor_x_ > new_line_len)
		{
			cursor_x_ = new_line_len;
		}

		adjust_cursor_column_to_desired(new_line_len);

		if (documet_to_workspace_y(cursor_y_) == window_.get_workspace_height())
		{
			// scroll one line down
			first_line_ ++;
			request_full_render();
		}
		else
		{
			// just move cursor
			request_cursor_update();
		}
	}
}

void editor::cursor_left()
{
	if (cursor_x_ > 0)
	{
		cursor_x_--;
		desired_cursor_column_ = document_x_to_column(cursor_y_, cursor_x_);
		int workspace_x = column_to_workspace_x(desired_cursor_column_);

		if (workspace_x < 0)
		{
			// scroll left
			first_column_ += workspace_x;
			request_full_render();
		}
		else
		{
			request_cursor_update();
		}
	}
}

void editor::cursor_right()
{
	int ll = doc_.line_length(cursor_y_);
	if (cursor_x_ < ll)
	{
		cursor_x_++;
		desired_cursor_column_ = document_x_to_column(cursor_y_, cursor_x_);
		int workspace_x = column_to_workspace_x(desired_cursor_column_);

		if(workspace_x >= window_.get_workspace_width())
		{
			// scroll right
			first_column_ += (window_.get_workspace_width() - workspace_x);
			request_full_render();
		}
		else
		{
			// just move cursor
			request_cursor_update();
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


int editor::column_to_workspace_x(unsigned column) const
{
	return column - first_column_;
}

int editor::documet_to_workspace_y(unsigned docy) const
{
	return docy - first_line_;
}

unsigned editor::workspace_to_document_x(unsigned wx) const
{
	return wx + first_column_;
}

unsigned editor::workspace_to_document_y(unsigned wy) const
{
	return wy + first_line_;
}

unsigned editor::document_x_to_column(unsigned docy, unsigned docx) const
{
	const document::document_line& line =  doc_.get_line(docy);

	if (docx > line.get_length())
		throw std::runtime_error("line character out of bounds");

	unsigned x = 0;
	const char* text = line.get_data();
	for(; docx > 0; docx--, text++)
	{
		if (*text == '\t')
			x = (x+tab_width_) - (x+tab_width_)%tab_width_;
		else
			x++;
	}
	return x;
}

void editor::insert_at_cursor(char c)
{
	document::document_line& line = doc_.get_line(cursor_y_);
	line.insert(cursor_x_, c);
	if (c == '\n')
	{
		cursor_x_ = 0;
		desired_cursor_column_ = 0;
		cursor_y_++;
	}
	else
	{
		cursor_x_++;
		desired_cursor_column_ = document_x_to_column(cursor_y_, cursor_x_);
	}
	doc_.parse_language();
	request_full_render();
}

}
