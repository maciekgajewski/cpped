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

bool editor::on_special_key(int key_code, const char* /*key_name*/)
{

	// hard-coded actions
	switch(key_code)
	{
		case KEY_UP:
			cursor_up();
			return true;
		case KEY_DOWN:
			cursor_down();
			return true;
		case KEY_LEFT:
			cursor_left();
			return true;
		case KEY_RIGHT:
			cursor_right();
			return true;
	}

	return false;
}

unsigned editor::on_sequence(const std::string& sequence)
{
	insert_at_cursor(sequence);
	return sequence.length();
}

bool editor::on_mouse(const MEVENT& event)
{
	// ?
	return false;
}

void editor::cursor_up()
{
	if (cursor_pos_.line > 0)
	{
		cursor_pos_.line--;
		unsigned new_line_len = doc_.line_length(cursor_pos_.line);
		if (cursor_pos_.column > new_line_len)
		{
			cursor_pos_.column = new_line_len;
		}

		adjust_cursor_column_to_desired(new_line_len);

		if (documet_to_workspace_y(cursor_pos_.line) ==-1)
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
	unsigned current_column = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	while(current_column < desired_cursor_column_ && cursor_pos_.column < new_line_len)
	{
		cursor_pos_.column++;
		current_column = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	}
	while(current_column > desired_cursor_column_ + tab_width_ && cursor_pos_.column > 0)
	{
		cursor_pos_.column--;
		current_column = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	}
}

void editor::ensure_cursor_visible()
{
	// y
	if (cursor_pos_.line < first_line_)
	{
		first_line_ = cursor_pos_.line;
	}
	else if (cursor_pos_.line > first_line_ + window_.get_workspace_height())
	{
		first_line_ = cursor_pos_.line - window_.get_workspace_height();
	}

	// x
	if (cursor_pos_.column < first_column_)
	{
		first_column_ = 0;
	}
	else if (cursor_pos_.column > first_column_ + window_.get_workspace_width())
	{
		first_column_ = cursor_pos_.column - window_.get_workspace_width();
	}
}

void editor::request_full_render()
{
	window_.render(doc_, first_column_, first_line_, tab_width_);
	request_cursor_update();
}

void editor::request_cursor_update()
{
	int column = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	int cx = column - int(first_column_);
	int cy = int(cursor_pos_.line )- int(first_line_);

	window_.update_status_line(cursor_pos_.line, cursor_pos_.column, column, doc_.get_last_parse_time());
	window_.refresh_cursor(cy, cx);
}

void editor::cursor_down()
{
	if (cursor_pos_.line < doc_.get_line_count()-1)
	{
		cursor_pos_.line++;
		unsigned new_line_len = doc_.line_length(cursor_pos_.line);
		if (cursor_pos_.column > new_line_len)
		{
			cursor_pos_.column = new_line_len;
		}

		adjust_cursor_column_to_desired(new_line_len);

		if (documet_to_workspace_y(cursor_pos_.line) == window_.get_workspace_height())
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
	if (cursor_pos_.column > 0)
	{
		cursor_pos_.column--;
		desired_cursor_column_ = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
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
	int ll = doc_.line_length(cursor_pos_.line);
	if (cursor_pos_.column < ll)
	{
		cursor_pos_.column++;
		desired_cursor_column_ = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
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
	assert(line.get_length() >= docx);
	for(; docx > 0; docx--, text++)
	{
		if (*text == '\t')
			x = (x+tab_width_) - (x+tab_width_)%tab_width_;
		else
			x++;
	}
	return x;
}

void editor::insert_at_cursor(const std::string& s)
{
	doc_.insert(cursor_pos_, s);
	doc_.parse_language();

	// calculate new cursor pos
	for(char c : s)
	{
		if (c == '\n')
		{
			cursor_pos_.column = 0;
			cursor_pos_.line++;
		}
		else
		{
			cursor_pos_.column++;
		}
	}

	desired_cursor_column_ = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	ensure_cursor_visible();
	request_full_render();
}

}
