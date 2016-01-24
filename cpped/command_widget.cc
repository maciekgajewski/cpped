#include "command_widget.hh"

namespace cpped {

command_widget::command_widget(project& pr, nct::window_manager& wm, nct::event_window* parent)
	: nct::event_window(wm, parent)
	, project_(pr)
	, editor_(wm, this)
{
	editor_.set_help_text("(ctrl-p) to enter comands, (ctrl-k) to naviagte");
	editor_.set_style(nct::style{COLOR_BLACK, COLOR_WHITE});
}

void command_widget::activate(const std::string& init)
{
	editor_.set_text(init);
	editor_.move_cursor_to_end();
	editor_.set_active();
}

void command_widget::on_activated()
{
	editor_.set_active();
}

void command_widget::on_resized()
{
	editor_.move(nct::position{0, 0}, nct::size{1, get_size().w});
}

bool command_widget::on_special_key(int key_code, const char* key_name)
{
	if (key_code == 27) // ESC
	{
		cancelled_signal();
		return true;
	}
}

}
