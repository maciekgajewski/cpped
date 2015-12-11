#include "editor_window.hh"

#include "styles.hh"

#include "document_lib/document.hh"

#include "nct/ncurses_window.hh"

#include <cstdio>

namespace cpped {

editor_window::editor_window(WINDOW* w, nct::event_dispatcher& ed, style_manager& sm, document::document& doc)
	: event_window(w, ed, 0, nullptr), styles_(sm), editor_(*this, doc)
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

void editor_window::render(document::document& doc, unsigned first_column, unsigned first_line, unsigned tab_width)
{
	clear();

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
	doc.for_lines(first_line, get_height(), [&](const document::document_line& line)
	{
		move(line_no, 0);

		// print line number
		std::snprintf(lineno, 32, fmt, first_line+line_no++);
		attr_print(styles_.line_numbers, lineno, left_margin_width_);

		// print line
		unsigned column = 0;
		line.for_each_token([&](const document::line_token& token)
		{
			unsigned begin = std::max(token.begin, first_column);
			unsigned end = std::min(get_workspace_width(), token.end-first_column);

			int attr = styles_.get_attr_for_token(token.type);

			column = render_text(attr, tab_width, first_column, column, line.get_data() + begin, line.get_data() + end);
		});
	});
}

void editor_window::refresh_cursor(int wy, int wx)
{
	int x = wx + left_margin_width_;
	int y = wy + top_margin_;

	if (x >= 0 && y >= 0 && x < get_workspace_width() && y < get_workspace_height())
	{
		::curs_set(1);
		move(y, x);
	}
	else
	{
		::curs_set(0); // hide cursor
	}

}

unsigned editor_window::render_text(attr_t attr, unsigned tab_width, unsigned first_column, unsigned phys_column, const char* begin, const char* end)
{
	set_attr_on(attr);
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
						put_visual_tab();
					else
						put_char(' ');
				}
			}
		}
		else
		{
			if (phys_column >= first_column)
				put_char(*begin);
			phys_column++;
		}
		begin++;
	}

	set_attr_off(attr);

	return phys_column;
}

void editor_window::put_visual_tab()
{
	if (visualise_tabs_)
	{
		set_attr(A_DIM);
		put_char('|'); // TODO maybe use some cool unicode char?
		unset_attr(A_DIM);
	}
	else
	{
		put_char(' ');
	}
}

void editor_window::update_status_line(unsigned docy, unsigned docx, unsigned column, std::chrono::high_resolution_clock::duration last_parse_time)
{
	move(get_height()-1, 0);
	clear_to_eol();
	char buf[32];

	// cursor pos. character under cursor
	std::snprintf(buf, 32, "%d : %d-%d ", docy+1, docx+1, column+1);
	print(buf);

	// last parse time
	using namespace std::literals::chrono_literals;
	move(get_height()-1, 20);
	std::snprintf(buf, 32, "%.2fms", 0.001 * last_parse_time/1us);
	print(buf);

}

unsigned editor_window::get_workspace_width() const
{
	if (get_width() < left_margin_width_)
		return 0;
	else
		return get_width() - left_margin_width_;
}

unsigned editor_window::get_workspace_height() const
{
	return get_height() - top_margin_ - bottom_margin_;
}

}
