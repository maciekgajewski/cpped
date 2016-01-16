#pragma once

#include "navigator_widget.hh"

#include "nct/event_window.hh"
#include "nct/status_message.hh"
#include "nct/fbuttons.hh"

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
	std::unique_ptr<editor_window> editor_;
	nct::fbuttons fbuttons_;
	navigator_widget navigator_;
	nct::fbutton_action_provider fbutton_provider_;

};

}
