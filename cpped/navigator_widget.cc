#include "navigator_widget.hh"

#include <set>

namespace cpped
{

navigator_widget::navigator_widget(project& pr, nct::event_dispatcher& ed, nct::event_window* parent)
	: event_window(ed, parent)
	, project_(pr)
	, editor_(ed, this)
{
	editor_.set_help_text("type here to navigate (ctrl-k)");
	editor_.enter_pressed.connect([=]() { parent->set_active(); });
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

}
