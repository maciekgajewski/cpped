#include "list_widget.hh"

#include "event_dispatcher.hh"

namespace nct {

list_widget::list_widget(event_dispatcher& ed, list_widget::event_window* parent)
	: event_window(ed, parent)
{
}

void list_widget::select_next()
{
	if (current_item_ < items_.size() - 1)
	{
		current_item_ ++;
		while (current_item_ > get_workspace_height() + first_line_)
		{
			first_line_++;
		}
		update();
	}
}

void list_widget::select_previous()
{
	if (current_item_ > 0)
	{
		current_item_--;
		while(current_item_ < first_line_)
		{
			first_line_--;
		}
		update();
	}
}

bool list_widget::on_special_key(int key_code, const char* key_name)
{
	switch(key_code)
	{
		case KEY_UP:
			select_previous();
			return true;
		case KEY_DOWN:
			select_next();
			return true;
	}

	return false;
}

void list_widget::on_shown()
{
	update();
}

void list_widget::update()
{
	if (!is_visible()) return;
	ncurses_window& window = get_ncurses_window();

	int normal_attr = COLOR_PAIR(get_palette().get_pair_for_colors(COLOR_CYAN, COLOR_BLACK));
	int help_attr = COLOR_PAIR(get_palette().get_pair_for_colors(COLOR_CYAN, COLOR_BLUE));
	int selected_normal_attr = COLOR_PAIR(get_palette().get_pair_for_colors(COLOR_YELLOW, COLOR_BLACK));
	int selected_help_attr = COLOR_PAIR(get_palette().get_pair_for_colors(COLOR_YELLOW, COLOR_BLUE));


	window.clear();
	window.set_background(normal_attr | ' ');

	unsigned items_to_show = std::min<unsigned>(items_.size(), window.get_height());

	for(unsigned line = 0; line < items_to_show; line++)
	{
		const list_item& item = items_[line+first_line_];

		window.move_cursor(position{int(line), 0});
		if (line+first_line_ == current_item_)
		{
			window.attr_fill_line(selected_normal_attr, ' ', line);
			window.move_cursor(position{int(line), 0});
			window.attr_print(selected_normal_attr, item.text);
			window.move_cursor(position{int(line), int(longest_text_+1)});
			window.attr_print(selected_help_attr, item.help_text);
		}
		else
		{
			window.attr_print(normal_attr, item.text);
			window.move_cursor(position{int(line), int(longest_text_+1)});
			window.attr_print(help_attr, item.help_text);
		}

	}

	refresh_window();
}

void list_widget::items_changed()
{
	// calculate content size
	longest_text_ = 0;
	unsigned longest_help = 0;

	for(const list_item& item : items_)
	{
		longest_text_ = std::max<unsigned>(longest_text_, item.text.length());
		longest_help = std::max<unsigned>(longest_help, item.help_text.length());
	}

	content_size_.h = items_.size();
	content_size_.w = longest_text_ + 1 + longest_help;
}

}
