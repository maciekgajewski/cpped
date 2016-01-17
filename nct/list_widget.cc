#include "list_widget.hh"

#include "window_manager.hh"

#include <boost/range/adaptor/filtered.hpp>

namespace nct {

struct contains_filter_functor
{
	const std::string& filter_;

	bool operator() (const list_widget::list_item& item) const
	{
		if (filter_.empty()) return true;
		else return item.text.find(filter_) != std::string::npos;
	}
};

list_widget::list_widget(window_manager& ed, list_widget::event_window* parent)
	: event_window(ed, parent)
{
}

void list_widget::set_filter(const std::string& filter)
{
	if (filter_ != filter)
	{
		filter_ = filter;
		current_item_ = 0;
		count_displayed_items();
		request_redraw();
	}
}

list_widget::list_item* list_widget::get_current_item()
{
	if (items_displayed_ > 0)
	{
		contains_filter_functor contains_filter_pred{filter_};
		auto range = boost::adaptors::filter(items_, contains_filter_pred);
		auto it = range.begin();
		std::advance(it, current_item_);
		return &*it;
	}
	else
	{
		return nullptr;
	}
}

unsigned list_widget::get_filtered_count() const
{
	return items_displayed_;
}

void list_widget::select_next()
{
	if (current_item_ < items_displayed_ - 1)
	{
		current_item_ ++;
		while (current_item_ >= get_workspace_height() + first_line_)
		{
			first_line_++;
		}
		request_redraw();
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
		request_redraw();
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
	request_redraw();
}

void list_widget::on_resized()
{
	request_redraw();
}

void list_widget::render(ncurses_window& surface)
{
	nct::style normal_style = nct::style{COLOR_CYAN, COLOR_BLACK};
	nct::style help_style = nct::style{COLOR_CYAN, COLOR_BLUE};
	nct::style selected_normal_style = nct::style{COLOR_YELLOW, COLOR_BLACK};
	nct::style selected_help_style = nct::style{COLOR_YELLOW, COLOR_BLUE};


	surface.clear();
	surface.set_background(normal_style, ' ');

	// predicate for selecting lines with filter
	contains_filter_functor contains_filter_pred{filter_};

	unsigned items_to_show = std::min<unsigned>(items_displayed_, surface.get_height());

	auto range = boost::adaptors::filter(items_, contains_filter_pred);
	auto it = range.begin();
	std::advance(it, first_line_);
	for(unsigned line = 0; line < items_to_show; line++)
	{
		const list_item& item = *it++;

		surface.move_cursor(position{int(line), 0});
		if (line+first_line_ == current_item_)
		{
			surface.style_fill_line(selected_normal_style, ' ', line);
			surface.move_cursor(position{int(line), 0});
			surface.style_print(selected_normal_style, item.text);
			surface.move_cursor(position{int(line), int(longest_text_+1)});
			surface.style_print(selected_help_style, item.help_text);
		}
		else
		{
			surface.style_print(normal_style, item.text);
			surface.move_cursor(position{int(line), int(longest_text_+1)});
			surface.style_print(help_style, item.help_text);
		}
	}

	request_redraw();
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

	count_displayed_items();

	content_size_.w = longest_text_ + 1 + longest_help;
}

void list_widget::count_displayed_items()
{
	contains_filter_functor contains_filter_pred{filter_};
	items_displayed_ = std::count_if(items_.begin(), items_.end(), contains_filter_pred);
	content_size_.h = items_displayed_;
}

}
