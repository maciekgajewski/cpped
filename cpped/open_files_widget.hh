#pragma once

#include <nct/list_widget.hh>

#include <boost/filesystem.hpp>
#include <boost/container/flat_set.hpp>

namespace cpped {

class edited_file;

class open_files_widget : public nct::list_widget
{
public:

	open_files_widget(nct::window_manager& wm, nct::event_window* parent);

	// Adds/selects file
	void file_opened(edited_file& file);

	void select_next_file();
	void select_previous_file();

	// signals
	boost::signals2::signal<void(edited_file& file)> file_selected_signal;

private:

	void add_file(edited_file& file);
	void select_file(edited_file& file);

	void on_selection_changed(const nct::list_widget::list_item& item);

	boost::container::flat_set<edited_file*> files_;
	bool signal_blocked_ = false;

};

}
