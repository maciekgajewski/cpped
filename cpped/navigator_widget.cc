#include "navigator_widget.hh"

namespace cpped
{

navigator_widget::navigator_widget(nct::event_dispatcher& ed, nct::event_window* parent)
	: event_window(ed, parent)
	, editor_(ed, this)
{
	editor_.set_help_text("type here to navigate (ctrl-k)");
	editor_.enter_pressed.connect([=]() { parent->set_active(); });

	nct::line_edit::completion_hint hints[] = {
		{"aaa", "thee a's"}, {"bbbbbbbbb", "Arbitrary number of bees"}, {"ccc", "I think there is a clothin shop chain named like this..."}
		};

	editor_.set_completion_hints(hints);
}

void navigator_widget::on_resized()
{
	editor_.move(nct::position{0, 0}, nct::size{1, get_size().w});
}

}
