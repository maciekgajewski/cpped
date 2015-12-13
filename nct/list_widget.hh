#pragma once

#include "event_window.hh"

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
	};

	list_widget(event_dispatcher& ed, event_window* parent);

	template<typename Container>
	void set_items(const Container& items);

	// size required to displaty all the conent
	size get_content_size() const { return content_size_; }

	// actions

	void select_next();
	void select_previous();

private:

	bool on_special_key(int key_code, const char* key_name) override;

	void on_shown() override;

	void update();
	void items_changed();

	// useful aread
	unsigned get_workspace_height() const { return get_size().h; }
	unsigned get_workspace_width() const { return get_size().w; }

	std::vector<list_item> items_;
	unsigned current_item_ = 0;
	unsigned first_line_ = 0;
	size content_size_;
	unsigned longest_text_ = 0;
};

template<typename Container>
void list_widget::set_items(const Container& items)
{
	items_.assign(std::begin(items), std::end(items));
	items_changed();
}


}
