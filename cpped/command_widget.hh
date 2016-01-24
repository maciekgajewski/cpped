#pragma once

#include "project.hh"

#include <nct/event_window.hh>
#include <nct/line_edit.hh>

namespace cpped
{

// Command edit widget. Wrpas line edit and provides aid for command entry
class command_widget final : public nct::event_window
{
public:

	command_widget(project& pr, nct::window_manager& wm, nct::event_window* parent);

	// Activates the widget, and sets initial text
	void activate(const std::string& init = {});

	boost::signals2::signal<void(boost::filesystem::path&)> file_selected_signal;
	boost::signals2::signal<void()> cancelled_signal;

private:

	void on_activated() override;
	void on_resized() override;
	bool on_special_key(int key_code, const char *key_name);

	project& project_;
	nct::line_edit editor_;
};

}
