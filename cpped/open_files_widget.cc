#include "open_files_widget.hh"

#include "edited_file.hh"

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

void open_files_widget::file_opened(edited_file& file)
{
	auto it = files_.find(&file);
	if (it == files_.end())
	{
		add_file(file);
	}
	else
	{
		select_file(file);
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

void open_files_widget::add_file(edited_file& file)
{
	files_.insert(&file);
	std::vector<nct::list_widget::list_item> items;
	items.reserve(files_.size());

	unsigned current_item_index = 0;
	for(edited_file* ef : files_)
	{
		if (ef == &file)
		{
			current_item_index = items.size();
		}
		const fs::path& p = ef->get_path();

		items.push_back({p.empty() ? ef->get_name() : p.filename().string(), {}, ef});
	}

	set_items(items);
	signal_blocked_ = true;
	select_item(current_item_index);
	signal_blocked_ = false;
}

void open_files_widget::select_file(edited_file& file)
{
	// TODO
}

void open_files_widget::on_selection_changed(const nct::list_widget::list_item& item)
{
	if (!signal_blocked_)
	{
		edited_file* file  = boost::any_cast<edited_file*>(item.data);
		file_selected_signal(*file);
	}
}

}
