#pragma once

#include "project.hh"
#include "commands.hh"
#include "command_context.hh"

#include <nct/event_window.hh>
#include <nct/line_edit.hh>

namespace cpped
{

// Command edit widget. Wrpas line edit and provides aid for command entry
class command_widget final : public nct::event_window, public command_context
{
public:

	command_widget(project& pr, nct::window_manager& wm, nct::event_window* parent);

	// Activates the widget, and sets initial text
	void activate(const std::string& init = {});

	boost::signals2::signal<void(const boost::filesystem::path&)> file_selected_signal;
	boost::signals2::signal<void()> cancelled_signal;

private:

	void on_activated() override;
	void on_deactivated() override;
	void on_resized() override;
	bool on_special_key(int key_code, const char *key_name);

	void on_text_changed(const std::string& text);
	void on_enter_pressed();

	// actions
	void cancel();

	// command context

	void open_file(const boost::filesystem::path& path) override;
	void show_hints(unsigned position, const hint_list& items) override;
	nct::list_widget::list_item* get_current_item() override;
	void set_text(const std::string& t) override;
	std::string get_text() const override { return editor_.get_text(); }
	const project& get_project() const override { return project_; }

	project& project_;
	nct::line_edit editor_;
	nct::list_widget hints_;
	std::unique_ptr<base_command> root_command_;
	bool ignore_signals_ = false;
};

}
