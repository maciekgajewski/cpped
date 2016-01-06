#include "main_window.hh"

#include "styles.hh"
#include "editor_window.hh"

#include <boost/filesystem.hpp>

namespace cpped {

namespace fs = boost::filesystem;

main_window::main_window(project& pr, nct::event_dispatcher& ed, style_manager& sm)
	: nct::event_window(ed, nullptr),
	project_(pr), style_(sm),
	status_message_receiver_([this](const std::string& s) { set_status_message(s); }),
	editor_(std::make_unique<editor_window>(pr, ed, sm, this)),
	fbuttons_(ed, this),
	navigator_(pr, ed, this)
{
	project_.status_signal.connect(
		[this](const std::string st)
		{
			if (st != project_status_)
			{
				project_status_ = st;
				update();
			}
		});

	navigator_.file_selected_signal.connect(
		[this](const fs::path& p)
		{
			auto& editor = get_current_editor();
			editor.set_active();
			editor.open_file(p);
		});

	fbutton_provider_.set_action(
		9 /* F10 */, "Quit", {});
}

void main_window::on_shown()
{
	update();
}

void main_window::on_resized()
{
	nct::size sz = get_size();
	editor_->move({0, 0}, {sz.h - 3, sz.w});
	fbuttons_.move({sz.h - 1, 0}, {1, sz.w});
	navigator_.move({sz.h - 2, 0}, {1, sz.w});
	update();
}

void main_window::on_activated()
{
	editor_->set_active();
}

bool main_window::on_special_key(int key_code, const char* key_name)
{
	static const std::string navigation = "^K";

	if (key_name == navigation)
	{
		navigator_.set_active();
		return true;
	}

	return false;
}

void main_window::update()
{
	// just draw a bar at the bottom
	if (!is_visible()) return;
	nct::ncurses_window& window = get_ncurses_window();

	nct::style style{COLOR_CYAN, COLOR_BLACK};
	window.horizontal_line(window.get_height()-3, 0, style, ' ', window.get_width());

	// status - left aligned
	window.move_cursor(window.get_height()-3, 0);
	window.style_print(style, status_);

	// project status - right aligned
	window.move_cursor(window.get_height()-3, window.get_width()-project_status_.size());
	window.style_print(style, project_status_);
}

void main_window::set_status_message(const std::string& st)
{
	if (st != status_)
	{
		status_ = st;
		update();
	}
}

}
