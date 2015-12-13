#pragma once

#include "project.hh"

#include "nct/event_window.hh"
#include "nct/line_edit.hh"

namespace cpped
{

// widget for code navigation,. Something like addressbar in a web broser. An inplementation of QtCreator's Ctrl-K tool.
class navigator_widget final : public nct::event_window
{
public:

	navigator_widget(project& pr, nct::event_dispatcher& ed, nct::event_window* parent);

	void on_activated() override;


private:

	virtual void on_resized() override;

	project& project_;
	nct::line_edit editor_;
};

}
