#include "command_widget.hh"
namespace cpped {

command_widget::command_widget(project& pr, nct::window_manager& wm, nct::event_window* parent)
	: nct::event_window(wm, parent)
	, command_context_{wm}
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

	command_context_.hint_list = &hints_;
}

void command_widget::activate(const std::string& init)
{
	root_command_ = make_root_command(command_context_);

	editor_.set_text(init);
	editor_.move_cursor_to_end();
	editor_.set_active();
	root_command_->on_text_changed(init);
}

void command_widget::on_activated()
{
	editor_.set_active();
}

void command_widget::on_resized()
{
	editor_.move(nct::position{0, 0}, nct::size{1, get_size().w});
	command_context_.editor_pos = nct::position{0, 0};
	command_context_.editor_width = get_size().w;
}

bool command_widget::on_special_key(int key_code, const char* key_name)
{
	if (key_code == 27) // ESC
	{
		hints_.hide();
		cancelled_signal();
		return true;
	}

	return false;
}

void command_widget::on_text_changed(const std::string& text)
{
	assert(root_command_);
	root_command_->on_text_changed(text);
}

void command_widget::on_enter_pressed()
{
	assert(root_command_);
	root_command_->on_enter_pressed();
}

}
