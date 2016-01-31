#pragma once

#include <nct/list_widget.hh>

#include <boost/filesystem.hpp>

namespace cpped {

class filesystem_widget : public nct::list_widget
{
public:

	filesystem_widget(nct::window_manager& wm, nct::event_window* parent);

	void set_directory(const boost::filesystem::path& path);

private:

	void update_files();

	boost::filesystem::path path_;
};

}
