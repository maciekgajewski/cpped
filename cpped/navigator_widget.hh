#pragma once

#include "nct/event_window.hh"
#include "nct/line_edit.hh"

namespace cpped
{

// widget for code navigation,. Something like addressbar in a web broser. An inplementation of QtCreator's Ctrl-K tool.
class navigator_widget final : public nct::event_window
{
public:

	navigator_widget(nct::event_dispatcher& ed, nct::event_window* parent);

	void on_activated() override { editor_.set_active(); }


private:

	virtual void on_resized() override;

	nct::line_edit editor_;
};

}
