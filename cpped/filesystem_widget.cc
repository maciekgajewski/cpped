#include "filesystem_widget.hh"

namespace cpped {

namespace fs = boost::filesystem;

filesystem_widget::filesystem_widget(nct::window_manager& wm, nct::event_window* parent)
	: nct::list_widget(wm, parent)
{
	set_title("File System");
	set_text_style(nct::style{COLOR_BLACK, COLOR_WHITE});
	set_selected_text_style(nct::style{COLOR_BLACK, COLOR_WHITE});

	path_ = fs::current_path();
	update_files();
}

void filesystem_widget::set_directory(const fs::path& path)
{
	assert(path.is_absolute());
	if (path != path_)
	{
		path_ = path;
		update_files();
	}
}

void filesystem_widget::update_files()
{
	std::vector<nct::list_widget::list_item> items;

	// add ..
	if (path_ != path_.root_path())
	{
		items.push_back(nct::list_widget::list_item{".."});
	}

	fs::directory_iterator it(path_);
	for(const fs::directory_entry& entry : it)
	{
		std::string name = entry.path().filename().string();
		if (entry.status().type() == fs::directory_file)
		{
			name += "/";
		}
		items.push_back(nct::list_widget::list_item{name});
	}

	set_items(items);
}

}
