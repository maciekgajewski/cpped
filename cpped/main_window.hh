#pragma once

#include "nct/event_window.hh"

#include <memory>

namespace cpped {

class style_manager;
class project;
class editor_window;

class main_window : public nct::event_window
{
public:
	main_window(project& pr, nct::event_dispatcher& ed, style_manager& sm);

	void on_shown() override;
	void on_resized() override;
	void on_activated() override;

	editor_window& get_current_editor() const { return *editor_; }

private:

	void update();

	std::string status_;

	project& project_;
	style_manager& style_;
	std::unique_ptr<editor_window> editor_;
};

}
