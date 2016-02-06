#include "main_window.hh"

#include "styles.hh"
#include "editor_window.hh"

#include "utils_lib/event_loop.hh"

#include <boost/filesystem.hpp>

namespace cpped {

namespace fs = boost::filesystem;

static bool is_yes(const std::string& answer)
{
	return answer.length() > 0 && (answer[0] == 'y' && answer[0] == 'Y');
}


main_window::main_window(project& pr, nct::window_manager& wm, style_manager& sm, edited_file& f)
	: nct::event_window(wm, nullptr),
	project_(pr), style_(sm),
	status_message_receiver_([this](const std::string& s) { set_status_message(s); }),
	main_splitter_(wm, this),
	open_file_list_(wm, &main_splitter_),
	filesystem_widget_(wm, &main_splitter_),
	editor_(std::make_unique<editor_window>(pr, wm, sm, f, &main_splitter_)),
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

	auto open_files_item = std::make_unique<nct::splitter_item>(main_splitter_, open_file_list_);
	auto filesystem_item = std::make_unique<nct::splitter_item>(main_splitter_, filesystem_widget_);
	auto left_panel = std::make_unique<nct::splitter_section>(main_splitter_, 30);
	left_panel->add_item(*open_files_item);
	left_panel->add_item(*filesystem_item);

	auto right_panel = std::make_unique<nct::splitter_item>(main_splitter_, *editor_);

	auto main_section = std::make_unique<nct::splitter_section>(main_splitter_);
	main_section->add_item(*left_panel);
	main_section->add_item(*right_panel);
	main_splitter_.set_main_section(*main_section, nct::splitter::horizontal);

	splitter_items_.push_back(std::move(open_files_item));
	splitter_items_.push_back(std::move(filesystem_item));
	splitter_items_.push_back(std::move(left_panel));
	splitter_items_.push_back(std::move(right_panel));
	splitter_items_.push_back(std::move(main_section));

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
	command_widget_.save_as_signal.connect(
		[this](const fs::path& p)
		{
			save_as(p);
		});

	open_file_list_.file_selected_signal.connect(
		[this](edited_file& ef)
		{
			auto& editor = get_current_editor();
			editor.set_active();
			editor.open_file(ef);
		});

	editor_->save_as_request_signal.connect(
		[this]() { command_widget_.activate("save-as "); });

	connect_project_to_open_files();

	fbutton_provider_.set_action(
				9 /* F10 */, "Quit", [this]() { quit(); });
}

void main_window::connect_project_to_open_files()
{
	project_.file_opened_signal.connect(
		[this](edited_file& ef) { open_file_list_.file_opened(ef); });

	std::vector<edited_file*> open_files;
	project_.get_all_open_files(std::back_inserter(open_files));
	for(edited_file* f : open_files)
	{
		open_file_list_.file_opened(*f);
	}
}

void main_window::open_file(const boost::filesystem::path& path)
{
	editor_->open_file(path);
	editor_->set_active();
	filesystem_widget_.set_directory(path.parent_path());
}

void main_window::save_as(const boost::filesystem::path& path)
{
	if (fs::exists(path))
	{
		auto answer = ask("File " + path.string() + " exists, overwrite? [y/N]");
		if (!is_yes(answer))
		{
			return;
		}
	}

	editor_->save_as(path);
	editor_->set_active();
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

template<typename Lambda>
struct OutputIt
{
	Lambda lambda_;

	OutputIt& operator *() { return *this; }
	void operator++() {}
	template<typename Val>
	void operator=(const Val& v) { lambda_(v); }
};

template<typename Lambda>
OutputIt<Lambda> iterator(Lambda l)
{
	return OutputIt<Lambda>{l};
}

void main_window::quit()
{
	bool has_unsaved_changes = false;
	project_.get_all_open_files(iterator([&](const edited_file* f)
	{
		if (f->get_document().has_unsaved_changes())
			has_unsaved_changes = true;
	}));

	if (has_unsaved_changes)
	{
		std::string answer = ask("There are unsaved files, quit? [y/N]");
		if (!is_yes(answer))
		{
			return;
		}
	}

	utils::event_loop::get_current()->stop();
}

std::string main_window::ask(const std::string& question)
{
	nct::ncurses_window dialog(command_widget_.get_position(), command_widget_.get_size());

	nct::style question_style(COLOR_YELLOW, COLOR_BLACK);
	nct::style answer_style(COLOR_BLACK, COLOR_WHITE);

	dialog.print({0, 0}, question_style, question);
	dialog.horizontal_line(0, question.length(), answer_style, ' ', dialog.get_width() - question.length());
	dialog.move_cursor({0, int(question.length())});
	dialog.set_timeout(-1);
	::echo();
	char buf[100];
	::wscanw(dialog.get_window(), "%s", buf);
	::noecho();
	return buf;
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
