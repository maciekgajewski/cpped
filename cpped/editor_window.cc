#include "editor_window.hh"

#include "styles.hh"

#include "document_lib/document.hh"

#include "nct/ncurses_window.hh"

#include <cstdio>

namespace cpped {

namespace fs = boost::filesystem;

editor_window::editor_window(project& pr, nct::event_dispatcher& ed, style_manager& sm, event_window* parent)
	: event_window(ed, parent), project_(pr), styles_(sm), editor_(*this)
	, navigator_(pr, ed, this)
{
	navigator_.file_selected_signal.connect(
		[this](const fs::path& p) { open_file(p); set_active(); });
}

unsigned editor_window::on_sequence(const std::string& s)
{
	// TODO handle any events belonging to me
	return editor_.on_sequence(s);
}

bool editor_window::on_special_key(int key_code, const char* key_name)
{
	static const std::string navigation = "^K";

	if (key_name == navigation)
	{
		navigator_.set_active();
		return true;
	}



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
	navigator_.move(nct::position{1, 5}, nct::size{1, get_size().w - 7});
	editor_.update();
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

	refresh_window();
}

void editor_window::refresh_cursor(int wy, int wx)
{
	if (!is_visible()) return;

	if (wx >= 0 && wy >= 0 && wx < get_workspace_width() && wy < get_workspace_height())
	{
		int x = wx + left_margin_width_;
		int y = wy + top_margin_;
		show_cursor({y,x});
	}
	else
	{
		hide_cursor();
	}

	refresh_window();
}

unsigned editor_window::render_text(nct::ncurses_window& window, attr_t attr, unsigned tab_width, unsigned first_column, unsigned phys_column, const char* begin, const char* end)
{
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
						window.put_char(' ' | attr);
				}
			}
		}
		else
		{
			if (phys_column >= first_column)
				window.put_char(*begin | attr);
			phys_column++;
		}
		begin++;
	}

	return phys_column;
}

void editor_window::put_visual_tab(nct::ncurses_window& window)
{
	if (visualise_tabs_)
	{
		window.put_char('|' | styles_.visual_tab); // TODO maybe use some cool unicode char?
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
	std::string file_name_string = info.file_name.string();
	int file_name_x = (window.get_width() - file_name_string.length()) / 2;
	window.move_cursor(0, file_name_x);
	window.print(info.file_name.string());
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

	refresh_window();
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

void editor_window::open_file(const boost::filesystem::path& file)
{
	// TODO modal dialog to save unsavedchanges in current document

	document::document& doc = project_.open_file(file);
	editor_.set_document(doc);
}

}
