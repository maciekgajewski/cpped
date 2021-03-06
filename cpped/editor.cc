#include "editor.hh"

#include "editor_window.hh"
#include "clipboard.hh"
#include "edited_file.hh"

#include "document_lib/document.hh"

namespace cpped {

editor::editor(editor_window& win, edited_file& f)
	: window_(win)
{
	set_document(f);
}

bool editor::on_special_key(int key_code, const char* key_name)
{

	// hard-coded actions
	switch(key_code)
	{
		// arrows
		case KEY_UP:
			arrow_up(); return true;
		case KEY_DOWN:
			arrow_down(); return true;
		case KEY_LEFT:
			arrow_left(); return true;
		case KEY_RIGHT:
			arrow_right(); return true;

		// arrows w/shift
		case KEY_SR:
			shift_arrow_up(); return true;
		case KEY_SF:
			shift_arrow_down(); return true;
		case KEY_SLEFT:
			shift_arrow_left(); return true;
		case KEY_SRIGHT:
			shift_arrow_right(); return true;

		// pgup/pgdown
		case KEY_PPAGE:
			pg_up(); return true;
		case KEY_NPAGE:
			pg_down(); return true;

		// baclspace/del
		case KEY_BACKSPACE:
			backspace(); return true;
		case KEY_DC:
			del(); return true;

		// home/end
		case KEY_HOME:
			home(); return true;
		case KEY_END:
			end(); return true;

		// enter/tab
		case '\n':
			insert_at_cursor("\n"); return true;
		case '\t':
			insert_at_cursor("\t"); return true;
	}

	static const std::string copy_key = "^C";
	static const std::string paste_key = "^V";
	static const std::string cut_key = "^X";

	if (key_name == copy_key)
	{
		copy();
	}
	else if (key_name == paste_key)
	{
		paste();
	}
	else if (key_name == cut_key)
	{
		cut();
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

void editor::set_document(edited_file& f)
{
	if (file_)
	{
		editor_state es;
		es.cursor_position = cursor_pos_;
		es.first_line = first_line_;
		file_->set_editor_state(es);
	}

	file_ = &f;

	tokens_udated_connection_.release().disconnect();
	tokens_udated_connection_ = file_->get_document().tokens_updated_signal.connect([this]() { on_document_tokens_updated(); });

	if (file_->get_editor_state())
	{
		const editor_state& es = *file_->get_editor_state();
		cursor_pos_ = es.cursor_position;
		first_line_ = es.first_line;
	}
	else
	{
		cursor_pos_ = {0, 0};
	}
	ensure_cursor_visible();
	update();
}

void editor::replace(const document::document_position& pos, unsigned len, const std::string& replacement)
{
	auto edit = get_document().edit();
	if (len > 0)
	{
		edit.remove_after(pos, len);
	}
	cursor_pos_ = edit.insert(pos, replacement);
	edit.commit(cursor_pos_);
	request_parsing();
}

void editor::cursor_up()
{
	if (cursor_pos_.line > 0)
	{
		cursor_pos_.line--;
		adjust_cursor_column_to_desired();

		if (documet_to_workspace_y(cursor_pos_.line) ==-1)
		{
			assert(first_line_ > 0);
			first_line_--;

		}
		request_full_render();
	}
}

void editor::adjust_cursor_column_to_desired()
{
	unsigned new_line_len = get_document().line_length(cursor_pos_.line);
	if (cursor_pos_.column > new_line_len)
	{
		cursor_pos_.column = new_line_len;
	}

	unsigned current_column = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	while(current_column < desired_cursor_column_ && cursor_pos_.column < new_line_len)
	{
		cursor_pos_.column++;
		current_column = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	}
	while(current_column > desired_cursor_column_ + settings_.tab_width && cursor_pos_.column > 0)
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
	window_.request_full_render();
	update_window_title();
}

editor::status_info editor::get_status_info() const
{
	int column = document_x_to_column(cursor_pos_.line, cursor_pos_.column);

	status_info info;
	info.docx = cursor_pos_.column;
	info.docy = cursor_pos_.line;
	info.column = column;
	info.status_text = "";
	info.file_name = file_->get_name();
	info.unsaved = get_document().has_unsaved_changes();
	return info;
}

nct::position editor::get_cursor_workspace_position() const
{
	int column = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	int cx = column - int(first_column_);
	int cy = int(cursor_pos_.line )- int(first_line_);

	return {cy, cx};
}

void editor::request_parsing()
{
	if (!parsing_disabled_)
	{
		file_->request_parsing(boost::none);
	}
}

void editor::update_window_title()
{
	std::string name = file_->get_name();
	if (file_->get_document().has_unsaved_changes())
	{
		window_.set_title(name + "*");
	}
	else
	{
		window_.set_title(name);
	}
}

void editor::cursor_down()
{
	if (cursor_pos_.line < get_document().get_line_count()-1)
	{
		cursor_pos_.line++;
		adjust_cursor_column_to_desired();

		if (documet_to_workspace_y(cursor_pos_.line) == window_.get_workspace_height())
		{
			// scroll one line down
			first_line_ ++;
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
		}
	}
}

void editor::cursor_right()
{
	int ll = get_document().line_length(cursor_pos_.line);
	if (cursor_pos_.column < ll)
	{
		cursor_pos_.column++;
		desired_cursor_column_ = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
		int workspace_x = column_to_workspace_x(desired_cursor_column_);

		if(workspace_x >= window_.get_workspace_width())
		{
			// scroll right
			first_column_ += (workspace_x - window_.get_workspace_width());
		}
	}
}

void editor::copy()
{
	if (selection_)
	{
		clipboard::set(get_document().get_range_content(*selection_));
	}
}

void editor::paste()
{
	if (!clipboard::empty())
	{
		insert_at_cursor(clipboard::get());
	}
}

void editor::cut()
{
	if (selection_)
	{
		clipboard::set(get_document().get_range_content(*selection_));
		auto edit = get_document().edit();
		edit.remove(*selection_);
		cursor_pos_ = selection_->start;
		edit.commit(cursor_pos_);

		request_full_render();
	}
}

void editor::arrow_up()
{
	cursor_up();
	selection_.reset();
	request_full_render();
}

void editor::arrow_down()
{
	cursor_down();
	selection_.reset();
	request_full_render();
}

void editor::arrow_left()
{
	cursor_left();
	selection_.reset();
	request_full_render();
}

void editor::arrow_right()
{
	cursor_right();
	selection_.reset();
	request_full_render();
}

template<typename Action>
void editor::shift_arrow(Action action)
{
	auto cursor_before = cursor_pos_;
	action();
	auto cursor_after = cursor_pos_;

	if (cursor_before != cursor_after)
	{
		if (selection_)
		{
			if (cursor_before == selection_->end)
				selection_->end = cursor_after;
			else
				selection_->start = cursor_after;

			if (selection_->end == selection_->start)
				selection_.reset();
		}
		else
		{
			selection_ = document::document_range{
				std::min(cursor_before, cursor_after),
				std::max(cursor_before, cursor_after)};
		}
	}
	request_full_render();
}

void editor::shift_arrow_up()
{
	shift_arrow([this]() { cursor_up(); });
}

void editor::shift_arrow_down()
{
	shift_arrow([this]() { cursor_down(); });
}

void editor::shift_arrow_left()
{
	shift_arrow([this]() { cursor_left(); });
}

void editor::shift_arrow_right()
{
	shift_arrow([this]() { cursor_right(); });
}

void editor::pg_up()
{
	unsigned page_height = window_.get_workspace_height();
	first_line_ = first_line_ > page_height ? first_line_ - page_height : 0;
	cursor_pos_.line = cursor_pos_.line > page_height ? cursor_pos_.line - page_height : 0;

	adjust_cursor_column_to_desired();
	request_full_render();
}

void editor::pg_down()
{
	unsigned page_height = window_.get_workspace_height();
	unsigned lines = get_document().get_line_count();
	if (page_height < lines)
		first_line_ = std::min(first_line_ + page_height, lines - page_height);

	cursor_pos_.line = std::min(cursor_pos_.line  + page_height, lines-1);

	adjust_cursor_column_to_desired();
	request_full_render();
}

void editor::backspace()
{
	// TODO any smart-unindenting goes here
	if (selection_)
	{
		auto edit = get_document().edit();
		edit.remove(*selection_);
		cursor_pos_ = selection_->start;
		edit.commit(cursor_pos_);
		selection_.reset();
	}
	else if (cursor_pos_ > document::document_position{0, 0})
	{
		auto edit = get_document().edit();
		cursor_pos_ = edit.remove_before(cursor_pos_, 1);
		edit.commit(cursor_pos_);
	}

	request_parsing();
	ensure_cursor_visible();
	request_full_render();
}

void editor::del()
{
	if (selection_)
	{
		auto edit = get_document().edit();
		edit.remove(*selection_);
		cursor_pos_ = selection_->start;
		edit.commit(cursor_pos_);
		selection_.reset();
	}
	else if (cursor_pos_ < get_document().get_last_position())
	{
		auto edit = get_document().edit();
		edit.remove_after(cursor_pos_, 1);
		edit.commit(cursor_pos_);
	}

	request_parsing();
	request_full_render();
}

void editor::home()
{
	cursor_pos_.column = 0;
	desired_cursor_column_ = 0;
	ensure_cursor_visible();
	request_full_render();
}

void editor::end()
{
	const document::document_line& line = get_document().get_line(cursor_pos_.line);
	cursor_pos_.column = line.get_length();
	desired_cursor_column_ = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	ensure_cursor_visible();
	request_full_render();
}

void editor::on_document_tokens_updated()
{
	unsigned errors = 0;
	unsigned warnings = 0;
	for(const document::diagnostic_message& d : get_document().get_diagnostics())
	{
		if (d.severity == document::problem_severity::error)
			errors++;
		if (d.severity == document::problem_severity::warning)
			warnings++;
	}

	std::string s = "Document parsed. Erros: " + std::to_string(errors) + ", warnings: " + std::to_string(warnings);
	window_.set_status(s);

	request_full_render();
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
	const document::document_line& line =  get_document().get_line(docy);

	if (docx > line.get_length())
		throw std::runtime_error("line character out of bounds");

	unsigned x = 0;
	const char* text = line.get_data();
	assert(line.get_length() >= docx);
	for(; docx > 0; docx--, text++)
	{
		if (*text == '\t')
			x = (x+settings_.tab_width) - (x+settings_.tab_width)%settings_.tab_width;
		else
			x++;
	}
	return x;
}

void editor::insert_at_cursor(const std::string& s)
{
	auto edit = get_document().edit();
	if (selection_)
	{
		edit.remove(*selection_);
		cursor_pos_ = selection_->start;
		selection_.reset();
	}
	cursor_pos_ = edit.insert(cursor_pos_, s);
	edit.commit(cursor_pos_);

	// TODO check if can complete here
	request_parsing();

	desired_cursor_column_ = document_x_to_column(cursor_pos_.line, cursor_pos_.column);
	ensure_cursor_visible();
	request_full_render();
}

document::document& editor::get_document()
{
	assert(file_);
	return file_->get_document();
}

const document::document& editor::get_document() const
{
	assert(file_);
	return file_->get_document();
}

}
