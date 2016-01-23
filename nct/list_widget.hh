#pragma once

#include "event_window.hh"

#include <boost/any.hpp>
#include <boost/signals2.hpp>

#include <string>
#include <vector>

namespace nct {


// a widget displaying list of items, allowing for navigation with arrows, selection with enter
class list_widget : public event_window
{
public:

	struct list_item
	{
		std::string text;
		std::string help_text;
		boost::any data;
	};

	list_widget(window_manager& wm, event_window* parent);

	template<typename Container>
	void set_items(const Container& items);

	// only items whihc text contains 'filter' will be displayed
	void set_filter(const std::string& filter);

	// size required to displaty all the conent
	size get_content_size() const { return content_size_; }

	list_item* get_current_item(); // return null if no currently selected item

	unsigned get_filtered_count() const;

	// actions

	void select_next();
	void select_previous();
	void select_item(unsigned index); // by oridinal index, after applying filter

	// signals
	boost::signals2::signal<void(const list_item&)> selection_changed_signal;

private:

	bool on_special_key(int key_code, const char* key_name) override;

	void on_shown() override;
	void on_resized() override;

	void render(ncurses_window& surface) override;
	void items_changed();

	// useful aread
	unsigned get_workspace_height() const { return get_size().h; }
	unsigned get_workspace_width() const { return get_size().w; }

	void count_displayed_items();

	std::vector<list_item> items_;
	unsigned current_item_ = 0;
	unsigned first_line_ = 0;
	size content_size_;
	unsigned longest_text_ = 0;
	std::string filter_;
	unsigned items_displayed_ = 0; // number of displayed items, after applying filter
};

template<typename Container>
void list_widget::set_items(const Container& items)
{
	items_.assign(std::begin(items), std::end(items));
	items_changed();
}


}
