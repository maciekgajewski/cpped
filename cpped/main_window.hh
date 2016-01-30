#pragma once

#include "command_widget.hh"
#include "open_files_widget.hh"

#include <nct/event_window.hh>
#include <nct/status_message.hh>
#include <nct/fbuttons.hh>
#include <nct/splitter.hh>

#include <memory>

namespace cpped {

class style_manager;
class project;
class editor_window;

class main_window : public nct::event_window
{
public:
	main_window(project& pr, nct::window_manager& ed, style_manager& sm);

	editor_window& get_current_editor() const { return *editor_; }
	void open_file(const boost::filesystem::path& file);

	const project& get_project() const { return project_; }

private:

	void on_resized() override;
	void on_activated() override;
	bool on_special_key(int key_code, const char *key_name) override;
	void render(nct::ncurses_window& surface) override;

	void set_status_message(const std::string& st);

	std::string project_status_;
	std::string status_;

	project& project_;
	style_manager& style_;
	nct::status_message_receiver status_message_receiver_;
	nct::splitter main_splitter_;
	open_files_widget open_file_list_;
	std::unique_ptr<editor_window> editor_;
	nct::fbuttons fbuttons_;
	command_widget command_widget_;
	nct::fbutton_action_provider fbutton_provider_;

	std::vector<std::unique_ptr<nct::splitter_item>> splitter_items_;
};

}
