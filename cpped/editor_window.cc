#include "editor_window.hh"

#include "styles.hh"

#include "document_lib/document.hh"

#include "nct/ncurses_window.hh"

#include <cstdio>

namespace cpped {

editor_window::editor_window(nct::event_dispatcher& ed, style_manager& sm, document::document& doc)
	: event_window(ed, nullptr), styles_(sm), editor_(*this, doc)
	, navigator_(ed, this)
{
}

unsigned editor_window::on_sequence(const std::string& s)
{
	// TODO handle any events belonging to me
	return editor_.on_sequence(s);
}

bool editor_window::on_special_key(int key_code, const char* key_name)
{
	// TODO handle any events belonging to me
	return editor_.on_special_key(key_code, key_name);
}

void editor_window::on_mouse(const MEVENT& event)
{
	// TODO translate event into workspace coords
	// send to editor
}

void editor_window::on_shown()
{
	editor_.update();
}

void editor_window::on_resized()
{
	navigator_.move(nct::position{0, 5}, nct::size{0, get_size().w - 5});
}

void editor_window::render(document::document& doc, unsigned first_column, unsigned first_line, unsigned tab_width)
{
	if (!is_visible()) return;
	nct::ncurses_window& window = get_ncurses_window();

	window.clear();

	int line_count_digits = 8;
	if (doc.get_line_count() < 10)
		line_count_digits = 1;
	else if (doc.get_line_count() < 100)
		line_count_digits = 2;
	else if (doc.get_line_count() < 1000)
		line_count_digits = 3;
	else if (doc.get_line_count() < 10000)
		line_count_digits = 4;

	left_margin_width_ = line_count_digits + 2;
	char fmt[32];
	std::snprintf(fmt, 32, " %%%dd ", line_count_digits);
	char lineno[32];

	// iterate over lines
	int line_no = 0;
	doc.for_lines(first_line, get_workspace_height(), [&](const document::document_line& line)
	{
		window.move_cursor(line_no + top_margin_, 0);
		line_no++;

		// print line number
		std::snprintf(lineno, 32, fmt, first_line+line_no);
		window.attr_print(styles_.line_numbers, lineno, left_margin_width_);

		// print line
		unsigned column = 0;
		line.for_each_token([&](const document::line_token& token)
		{
			unsigned begin = std::max(token.begin, first_column);
			unsigned end = std::min(get_workspace_width(), token.end-first_column);

			int attr = styles_.get_attr_for_token(token.type);

			column = render_text(window, attr, tab_width, first_column, column, line.get_data() + begin, line.get_data() + end);
		});
	});

	window.redraw();
	window.no_out_refresh();
}

void editor_window::refresh_cursor(int wy, int wx)
{
	if (!is_visible()) return;
	nct::ncurses_window& window = get_ncurses_window();

	int x = wx + left_margin_width_;
	int y = wy + top_margin_;

	if (x >= 0 && y >= 0 && x < get_workspace_width() && y < get_workspace_height())
	{
		show_cursor({y,x});
	}
	else
	{
		hide_cursor();
	}

	window.redraw();
	window.no_out_refresh();
}

unsigned editor_window::render_text(nct::ncurses_window& window, attr_t attr, unsigned tab_width, unsigned first_column, unsigned phys_column, const char* begin, const char* end)
{
	window.set_attr_on(attr);
	unsigned last_column = get_workspace_width() + first_column;

	while(begin != end && phys_column != last_column)
	{
		if(*begin == '\t')
		{
			// render tab
			unsigned w = tab_width - phys_column%tab_width;
			for(unsigned c = 0; c < w && phys_column != last_column; c++, phys_column++)
			{
				if (phys_column >= first_column)
				{
					if (w == tab_width && c == 0) // first char of full tab
						put_visual_tab(window);
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

void editor_window::put_visual_tab(nct::ncurses_window& window)
{
	if (visualise_tabs_)
	{
		window.set_attr_on(A_DIM);
		window.put_char('|'); // TODO maybe use some cool unicode char?
		window.set_attr_off(A_DIM);
	}
	else
	{
		window.put_char(' ');
	}
}

void editor_window::update_status_info(const status_info& info)
{
	if (!is_visible()) return;
	nct::ncurses_window& window = get_ncurses_window();

	window.set_attr_on(styles_.status);

	// top line
	// window.horizontal_line(0,0, WACS_D_HLINE, window.get_width()); doesn't work on xfce (?)
	window.horizontal_line(0, 0, ACS_HLINE, window.get_width());
	int file_name_x = (window.get_width() - info.file_name.length()) / 2;
	window.move_cursor(0, file_name_x);
	window.print(info.file_name);
	if (info.unsaved)
		window.print("*");


	// bottom line

	window.move_cursor(window.get_height()-1, 0);
	window.clear_to_eol();
	char buf[32];

	// cursor pos. character under cursor
	std::snprintf(buf, 32, "%d : %d-%d ", info.docy+1, info.docx+1, info.column+1);
	window.print(buf);

	// status text
	window.move_cursor(window.get_height()-1, 20);
	window.print(info.status_text);

	window.redraw();
	window.no_out_refresh();
}

unsigned editor_window::get_workspace_width() const
{
	if (get_size().w < left_margin_width_)
		return 0;
	else
		return get_size().w - left_margin_width_;
}

unsigned editor_window::get_workspace_height() const
{
	return get_size().h - top_margin_ - bottom_margin_;
}

}
