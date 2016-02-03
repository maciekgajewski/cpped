#include "open_files_widget.hh"

#include "edited_file.hh"

#include <boost/range/algorithm.hpp>

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
	file.status_changed_signal.connect(
		[this]() { update_files(); });
	update_files();
	select_file(file);
}

void open_files_widget::select_file(edited_file& file)
{
	auto it = boost::find(files_, &file);
	assert(it != files_.end());
	select_item(files_.index_of(it));
}

void open_files_widget::update_files()
{
	nct::list_widget::list_item* current = get_current_item();

	std::vector<nct::list_widget::list_item> items;
	items.reserve(files_.size());

	int current_item_index = -1;
	for(edited_file* ef : files_)
	{
		if (current && ef == boost::any_cast<edited_file*>(current->data))
		{
			current_item_index = items.size();
		}
		const fs::path& p = ef->get_path();

		std::string text = p.empty() ? ef->get_name() : p.filename().string();
		if (ef->get_document().has_unsaved_changes())
		{
			text += "*";
		}

		items.push_back({text, {}, ef});
	}

	set_items(items);

	if(current_item_index >= 0)
	{
		signal_blocked_ = true;
		select_item(current_item_index);
		signal_blocked_ = false;
	}
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
