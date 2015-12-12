#include "navigator_widget.hh"

namespace cpped
{

navigator_widget::navigator_widget(nct::event_dispatcher& ed, nct::event_window* parent)
	: event_window(ed, parent)
	, editor_(ed, this)
{
	editor_.set_help_text("type here to navigate (ctrl-k)");
}

void navigator_widget::on_resized()
{
	editor_.move(nct::position{0, 0}, nct::size{1, get_size().w});
}

}
