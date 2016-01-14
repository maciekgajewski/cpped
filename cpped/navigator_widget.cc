#include "navigator_widget.hh"

#include <set>

namespace cpped
{

navigator_widget::navigator_widget(project& pr, nct::window_manager& ed, nct::event_window* parent)
	: event_window(ed, parent)
	, project_(pr)
	, editor_(ed, this)
{
	editor_.set_help_text("type here to navigate (ctrl-k)");
	editor_.set_style(nct::style{COLOR_BLACK, COLOR_WHITE});
	editor_.hint_selected_signal.connect([this](const nct::line_edit::completion_hint& h) { on_hint_selected(h); });
}

void navigator_widget::on_activated()
{
	// build the set of sefault hints
	std::set<boost::filesystem::path> files;

	project_.get_all_project_files(std::inserter(files, files.end()));
	project_.get_all_open_files(std::inserter(files, files.end()));

	std::vector<nct::line_edit::completion_hint> file_hints;
	file_hints.reserve(files.size());
	std::transform(files.begin(), files.end(), std::back_inserter(file_hints),
			[&](const boost::filesystem::path& path)
			{
				return nct::line_edit::completion_hint{path.filename().string(), path.string()};
			});

	editor_.set_completion_hints(file_hints);
	editor_.set_active();
}

void navigator_widget::on_resized()
{
	editor_.move(nct::position{0, 0}, nct::size{1, get_size().w});
}

void navigator_widget::on_hint_selected(const nct::line_edit::completion_hint& hint)
{
	// exit point - something has been selected
	// TODO need to distinguis between different hint list types

	editor_.set_text(std::string());

	boost::filesystem::path p(hint.help_text);
	file_selected_signal(p);
}

}
