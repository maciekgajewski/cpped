#include "editor_window.hh"

#include "styles.hh"

#include "document_lib/document.hh"

#include "nct/ncurses_window.hh"

#include <cstdio>

namespace cpped {

namespace fs = boost::filesystem;


struct text_renderer
{
	// fixed
	unsigned first_column_;
	const editor_settings& settings_;
	nct::ncurses_window& window_;
	unsigned workspace_width_;
	const boost::optional<document::document_range>& selection_;
	style_manager& styles_;

	// volatile
	unsigned phys_column_ = 0;
	document::document_position position_;

	void render_text(const nct::style& style, const char* begin, const char* end);
	void put_visual_tab();
	void put_char(const nct::style& style, char c);
};

void text_renderer::render_text(
	const nct::style& style,
	const char* begin, const char* end)
{
	unsigned last_column = workspace_width_ + first_column_;

	while(begin != end && phys_column_ != last_column)
	{
		if(*begin == '\t')
		{
			// render tab
			unsigned w = settings_.tab_width - phys_column_%settings_.tab_width;
			for(unsigned c = 0; c < w && phys_column_ != last_column; c++, phys_column_++)
			{
				if (phys_column_ >= first_column_)
				{
					if (w == settings_.tab_width && c == 0) // first char of full tab
						put_visual_tab();
					else
						put_char(style, ' ');
				}
			}
		}
		else
		{
			if (phys_column_ >= first_column_ && phys_column_ < last_column)
				put_char(style, *begin);
			phys_column_++;
		}
		position_.column++;
		begin++;
	}
}

void text_renderer::put_visual_tab()
{
	if (settings_.visualize_tab)
	{
		put_char(styles_.visual_tab, '|'); // TODO maybe use some cool unicode char?
	}
	else
	{
		put_char(styles_.plain, ' ');
	}
}

void text_renderer::put_char(const nct::style& style, char c)
{
	if (selection_ && position_ < selection_->end && position_ >= selection_->start)
	{
		// TODO invert style
		window_.put_char(nct::style{COLOR_WHITE, COLOR_BLACK}, c);
	}
	else
	{
		window_.put_char(style, c);
	}
}


editor_window::editor_window(project& pr, nct::event_dispatcher& ed, style_manager& sm, event_window* parent)
	: event_window(ed, parent), project_(pr), styles_(sm), editor_(*this)
	, navigator_(pr, ed, this)
	, completer_(pr, ed, this)
{
	navigator_.file_selected_signal.connect(
		[this](const fs::path& p) { open_file(p); set_active(); });

	completer_.completion_cancelled_signal.connect(
		[this]() { editor_.enable_parsing(); set_active(); });
	completer_.completion_signal.connect(
		[this](const document::document_position& pos, unsigned len, const std::string& text)
		{
			editor_.enable_parsing();
			editor_.replace(pos, len, text);
			set_active();
		});
}

unsigned editor_window::on_sequence(const std::string& s)
{
	// TODO handle any events belonging to me
	return editor_.on_sequence(s);
}

bool editor_window::on_special_key(int key_code, const char* key_name)
{
	static const std::string navigation = "^K";
	static const std::string complete = "^@";

	if (key_name == navigation)
	{
		navigator_.set_active();
		return true;
	}

	if (key_name == complete)
	{
		const document::document* doc = editor_.get_document();
		if (doc)
		{
			editor_.disable_parsing();
			document::document_position cursor_pos = editor_.get_cursor_position();
			completer_.activate(*doc, cursor_pos, cursor_pos_);
			return true;
		}
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

void editor_window::render(document::document& doc,
		unsigned first_column,
		unsigned first_line,
		const editor_settings& settings,
		const boost::optional<document::document_range>& selection)
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
	char lineno_buf[32];

	text_renderer renderer{first_column, settings, window, get_workspace_width(), selection, styles_};

	// iterate over lines
	unsigned line_no = 0;
	doc.for_lines(first_line, get_workspace_height(), [&](const document::document_line& line)
	{
		window.move_cursor(line_no + top_margin_, 0);

		// print line number
		std::snprintf(lineno_buf, 32, fmt, first_line+line_no+1);
		window.style_print(styles_.line_numbers, lineno_buf, left_margin_width_);

		// print line
		renderer.phys_column_ = 0;
		renderer.position_ = {line_no, 0};
		line.for_each_token([&](const document::line_token& token)
		{
			nct::style style = styles_.get_style_for_token(token.type);
			renderer.render_text(
				style,
				line.get_data() + token.begin,
				line.get_data() + token.end);
		});

		line_no++;
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
		cursor_pos_ = {y, x};
		show_cursor(cursor_pos_);
	}
	else
	{
		hide_cursor();
	}

	refresh_window();
}

void editor_window::update_status_info(const status_info& info)
{
	if (!is_visible()) return;
	nct::ncurses_window& window = get_ncurses_window();

	window.set_style_on(styles_.status);

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
	set_status("Document loaded");
}

}
