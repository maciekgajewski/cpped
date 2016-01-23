#include "open_files_widget.hh"

namespace cpped {

namespace fs = boost::filesystem;

open_files_widget::open_files_widget(nct::window_manager& wm, nct::event_window* parent)
	:nct::list_widget(wm, parent)
{
	selection_changed_signal.connect(
		[this](const nct::list_widget::list_item& i) { on_selection_changed(i); });

	set_text_style(nct::style{COLOR_BLACK, COLOR_WHITE});
	set_selected_text_style(nct::style{COLOR_YELLOW, COLOR_BLACK});
	set_title("Open files");
}

void open_files_widget::file_opened(const fs::path& path)
{
	auto it = files_.find(path);
	if (it == files_.end())
	{
		add_file(path);
	}
	else
	{
		select_file(path);
	}
}

void open_files_widget::select_next_file()
{
	select_next();
}

void open_files_widget::select_previous_file()
{
	select_previous();
}

void open_files_widget::add_file(const fs::path& path)
{
	files_.insert(path);
	std::vector<nct::list_widget::list_item> items;
	items.reserve(files_.size());

	unsigned current_item_index = 0;
	for(const auto& p : files_)
	{
		if (p == path)
		{
			current_item_index = items.size();
		}
		items.push_back({p.filename().string(), {}, p});
	}

	set_items(items);
	signal_blocked_ = true;
	select_item(current_item_index);
	signal_blocked_ = false;
}

void open_files_widget::select_file(const fs::path& path)
{
	// TODO
}

void open_files_widget::on_selection_changed(const nct::list_widget::list_item& item)
{
	if (!signal_blocked_)
	{
		fs::path path = boost::any_cast<fs::path>(item.data);
		file_selected_signal(path);
	}
}

}
