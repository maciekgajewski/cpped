#pragma once

#include <nct/list_widget.hh>

#include <boost/filesystem.hpp>
#include <boost/container/flat_set.hpp>

namespace cpped {

class open_files_widget : public nct::list_widget
{
public:

	open_files_widget(nct::window_manager& wm, nct::event_window* parent);

	// Adds/selects file
	void file_opened(const boost::filesystem::path& path);

	void select_next_file();
	void select_previous_file();

	// signals
	boost::signals2::signal<void(const boost::filesystem::path&)> file_selected_signal;

private:

	void add_file(const boost::filesystem::path& path);
	void select_file(const boost::filesystem::path& path);

	void on_selection_changed(const nct::list_widget::list_item& item);

	boost::container::flat_set<boost::filesystem::path> files_;
	bool signal_blocked_ = false;

};

}
