#include "command_widget.hh"

#include "main_window.hh"

namespace cpped {

namespace fs = boost::filesystem;

command_widget::command_widget(project& pr, nct::window_manager& wm, nct::event_window* parent)
	: nct::event_window(wm, parent)
	, project_(pr)
	, editor_(wm, this)
	, hints_(wm, this)
{
	editor_.set_help_text("(ctrl-p) to enter comands, (ctrl-k) to naviagte");
	editor_.set_style(nct::style{COLOR_BLACK, COLOR_WHITE});
	editor_.text_changed_signal.connect(
		[this](const std::string& t) { on_text_changed(t); });
	editor_.enter_pressed_signal.connect(
		[this]() { on_enter_pressed(); });

	hints_.hide();
}

void command_widget::activate(const std::string& init)
{
	root_command_ = make_root_command(*this);

	editor_.set_text(init);
	editor_.move_cursor_to_end();
	editor_.set_active();
	root_command_->on_text_changed(init);
}

void command_widget::on_activated()
{
	editor_.set_active();
}

void command_widget::on_deactivated()
{
	hints_.hide();
}

void command_widget::on_resized()
{
	editor_.move(nct::position{0, 0}, nct::size{1, get_size().w});
}

bool command_widget::on_special_key(int key_code, const char* key_name)
{
	switch(key_code)
	{
		case 27: // ESC
			hints_.hide();
			cancelled_signal();
			return true;
		case KEY_UP:
			if (hints_.is_visible())
			{
				hints_.select_previous();
			}
			return true;
		case KEY_DOWN:
			if (hints_.is_visible())
			{
				hints_.select_next();
			}
			return true;
	}

	return false;
}

void command_widget::on_text_changed(const std::string& text)
{
	if (!ignore_signals_)
	{
		assert(root_command_);
		root_command_->on_text_changed(text);
	}
}

void command_widget::on_enter_pressed()
{
	assert(root_command_);
	root_command_->on_enter_pressed();
}

void command_widget::show_hints(unsigned position, const std::vector<nct::list_widget::list_item>& items)
{
	if (items.empty())
	{
		hints_.hide();
	}
	else
	{
		hints_.set_items(items);

		nct::size content_size = hints_.get_content_size();
		nct::size sz;
		sz.h = std::min<int>(content_size.h, editor_.get_global_position().y);
		sz.w = std::min<int>(content_size.w, editor_.get_size().w - position);

		nct::position pos;
		pos.x = editor_.get_position().x + position;
		pos.y = editor_.get_position().y - sz.h;
		hints_.move(pos, sz);
		hints_.show();
	}
}

void command_widget::open_file(const boost::filesystem::path& path)
{
	cancel();
	file_selected_signal(path);
}

nct::list_widget::list_item*command_widget::get_current_item()
{
	return hints_.get_current_item();
}

void command_widget::set_text(const std::string& t)
{
	editor_.set_text(t);
	editor_.move_cursor_to_end();
}

void command_widget::cancel()
{
	ignore_signals_ = true;
	editor_.set_text({});
	hints_.hide();
	ignore_signals_ = false;
}

}
