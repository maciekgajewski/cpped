#include "main_window.hh"

#include "styles.hh"
#include "editor_window.hh"

#include "utils_lib/event_loop.hh"

#include <boost/filesystem.hpp>

namespace cpped {

namespace fs = boost::filesystem;

main_window::main_window(project& pr, nct::window_manager& wm, style_manager& sm)
	: nct::event_window(wm, nullptr),
	project_(pr), style_(sm),
	status_message_receiver_([this](const std::string& s) { set_status_message(s); }),
	main_splitter_(wm, this),
	open_file_list_(wm, &main_splitter_),
	editor_(std::make_unique<editor_window>(pr, wm, sm, &main_splitter_)),
	fbuttons_(wm, this),
	command_widget_(pr, wm, this)
{
	project_.status_signal.connect(
		[this](const std::string st)
		{
			if (st != project_status_)
			{
				project_status_ = st;
				request_redraw();
			}
		});

	main_splitter_.set_fixed(0, &open_file_list_, 30);
	main_splitter_.set_stretching(1, editor_.get());

	command_widget_.file_selected_signal.connect(
		[this](const fs::path& p)
		{
			open_file(p);
		});
	command_widget_.cancelled_signal.connect(
		[this]()
		{
			auto& editor = get_current_editor();
			editor.set_active();
		});

	open_file_list_.file_selected_signal.connect(
		[this](const fs::path& p)
		{
			auto& editor = get_current_editor();
			editor.set_active();
			editor.open_file(p);
		});

	fbutton_provider_.set_action(
				9 /* F10 */, "Quit", []() { utils::event_loop::get_current()->stop(); });
}

void main_window::open_file(const boost::filesystem::path& file)
{
	editor_->open_file(file);
	editor_->set_active();
	open_file_list_.file_opened(file);
}

void main_window::on_resized()
{
	nct::size sz = get_size();
	main_splitter_.move({0, 0}, {sz.h - 3, sz.w});
	fbuttons_.move({sz.h - 1, 0}, {1, sz.w});
	command_widget_.move({sz.h - 2, 0}, {1, sz.w});
	request_redraw();
}

void main_window::on_activated()
{
	editor_->set_active();
}

bool main_window::on_special_key(int key_code, const char* key_name)
{
	static const std::string command = "^P";
	static const std::string navigation = "^K";
	static const std::string find = "^F";
	static const std::string next_file = "^]";
	static const std::string prev_file = "^[";

	if (key_name == navigation)
	{
		command_widget_.activate("goto ");
		return true;
	}
	else if (key_name == command)
	{
		command_widget_.activate();
		return true;
	}
	else if (key_name == find)
	{
		command_widget_.activate("find ");
		return true;
	}
	else if (key_name == next_file)
	{
		open_file_list_.select_next_file();
		return true;
	}
	else if (key_name == next_file)
	{
		open_file_list_.select_next_file();
		return true;
	}
	else if (key_name == prev_file)
	{
		open_file_list_.select_previous_file();
		return true;
	}

	if (fbuttons_.try_special_key(key_code))
	{
		return true;
	}

	return false;
}

void main_window::render(nct::ncurses_window& surface)
{
	// just draw a bar at the bottom
	nct::style style{COLOR_WHITE, COLOR_BLACK};
	surface.horizontal_line(surface.get_height()-3, 0, style, ' ', surface.get_width());

	// status - left aligned
	surface.move_cursor(surface.get_height()-3, 0);
	surface.style_print(style, status_);

	// project status - right aligned
	surface.move_cursor(surface.get_height()-3, surface.get_width()-project_status_.size());
	surface.style_print(style, project_status_);
}

void main_window::set_status_message(const std::string& st)
{
	if (st != status_)
	{
		status_ = st;
		request_redraw();
	}
}

}
