#include "splitter.hh"

namespace nct {

splitter::splitter(window_manager& wm, event_window* parent)
	: nct::event_window(wm, parent)
{
}

void splitter::set_main_section(splitter_section& section, splitter::direction dir)
{
	main_section_ = &section;
	direction_ = dir;
	request_redraw();
}

void splitter::on_resized()
{
	if (main_section_)
	{
		main_section_->recalc_size(direction_, get_size());
	}
}

void splitter::render(nct::ncurses_window& surface)
{
	if (main_section_)
		main_section_->render(direction_, surface);
}

splitter_item::splitter_item(splitter& sp, event_window& window)
	: splitter_(sp), geometry_(geometry::stretching), window_(&window)
{
	window.set_parent(&sp);
}

splitter_item::splitter_item(splitter& sp, event_window& window, unsigned preferred_size)
	: splitter_(sp), geometry_(geometry::fixed), preferred_size_(preferred_size), window_(&window)
{
	window.set_parent(&sp);
}

bool splitter_item::is_visible() const
{
	if (visible_)
	{
		if (window_)
			return window_->is_visible();
		else
			return true;
	}
	else
	{
		return false;
	}
}

void splitter_item::apply_size(splitter::direction, const position& pos, const size& sz)
{
	assert(window_);
	nct::position window_pos = pos;
	nct::size window_sz = sz;

	window_pos.y += 1; // leave space for titlebar
	window_sz.h -= 1;

	window_->move(window_pos, window_sz);
}

splitter_section::splitter_section(splitter& sp)
	: splitter_item(sp)
{
}

splitter_section::splitter_section(splitter& sp, unsigned preferred_size)
	: splitter_item(sp, preferred_size)
{
}

void splitter_item::render(splitter::direction dir, ncurses_window& surface)
{
	assert(window_);

	const nct::position& pos = window_->get_position();
	const nct::size& sz = window_->get_size();

	surface.horizontal_line(pos.y-1, pos.x, ACS_HLINE, sz.w);

	const std::string& title = window_->get_title();

	if (title.length() < sz.w)
	{
		surface.move_cursor(pos.y -1, pos.x + (sz.w-title.length())/2);
		surface.print(title);
	}
}

static splitter::direction opposite(splitter::direction d)
{
	return d == splitter::direction::horizontal ?
		splitter::direction::vertical
		: splitter::direction::horizontal;
}

void splitter_section::render(splitter::direction dir, ncurses_window& surface)
{
	for(splitter_item* item : items_)
	{
		if (item->is_visible())
		{
			item->render(opposite(dir), surface);
		}
	}

	if (dir == splitter::direction::horizontal)
	{
		render_horizontal(surface);
	}
	else
	{
		render_vertical(surface);
	}
}

void splitter_section::add_item(splitter_item& item)
{
	items_.push_back(&item);
}

void splitter_section::recalc_size(splitter::direction dir, const size& sz)
{
	if (dir == splitter::direction::horizontal)
		recalc_size<splitter::direction::horizontal>(sz);
	else
		recalc_size<splitter::direction::vertical>(sz);
}

void splitter_section::apply_size(splitter::direction dir, const position& pos, const size& sz)
{
	position_ = pos;
	recalc_size(dir, sz);
}

void splitter_section::draw_horizontal_partition(unsigned x, ncurses_window& surface, splitter_item& left, splitter_item& right)
{
	surface.move_cursor(position_.y, position_.x + x);
	surface.put_char(ACS_TTEE);
	surface.vertical_line(position_.y + 1, position_.x + x, ACS_VLINE, size_.h-1);
}

void splitter_section::render_horizontal(ncurses_window& surface)
{
	splitter_item* left = nullptr;
	unsigned x = 0;
	for(splitter_item* item : items_)
	{
		if (item->is_visible())
		{
			if (left)
			{
				draw_horizontal_partition(x, surface, *left, *item);
			}
			x += item->size_;
			left = item;
		}
	}
}

void splitter_section::render_vertical(ncurses_window& surface)
{
	// nothing to do, window titles suffice
	// One may try to draw Ts and + if one of the items is splitter...
}

template<splitter::direction DIR> unsigned get_total(const nct::size& sz, unsigned item_count);
template<> unsigned get_total<splitter::direction::horizontal>(const nct::size& sz, unsigned item_count)
{
	return sz.w - (item_count > 0 ? (item_count-1) : 0); // substract 1 for each separator between items
}
template<> unsigned get_total<splitter::direction::vertical>(const nct::size& sz, unsigned item_count)
{
	return sz.h - item_count; // substract 1 for each item's title bar
}

template<splitter::direction DIR> double get_mulitiplier(unsigned total);
template<> double get_mulitiplier<splitter::direction::horizontal>(unsigned total)
{
	return total >= 80 ? 1.0 : total/80.0;
}
template<> double get_mulitiplier<splitter::direction::vertical>(unsigned total)
{
	return total >= 25 ? 1.0 : total/25.0;
}

template<splitter::direction DIR>
void splitter_section::recalc_size(const nct::size& sz)
{
	size_ = sz;

	// count visible items
	unsigned total_fixed_size = 0;
	unsigned stretching_items = 0;
	unsigned all_visible_items = 0;
	for(splitter_item* item : items_)
	{
		assert(item);
		if (item->is_visible())
		{
			all_visible_items++;

			if (item->geometry_ == geometry::fixed)
				total_fixed_size += item->preferred_size_;
			else
				stretching_items++;
		}
	}

	unsigned total = get_total<DIR>(sz, all_visible_items);

	// calculate sizes of fixed items
	double multiplier = get_mulitiplier<DIR>(total); // 1.0 for large windows, <1.0 for small windows
	if (total_fixed_size > total)
	{
		multiplier *= double(total_fixed_size)/total;
	}

	unsigned space_left_for_stretching = total;
	for(splitter_item* item : items_)
	{
		if (item->is_visible() && item->geometry_ == geometry::fixed)
		{
			item->size_ = std::floor(item->preferred_size_ * multiplier);
			space_left_for_stretching -= item->size_;
		}
	}

	// calculate sizes of stretching items
	if (stretching_items > 0)
	{
		unsigned stretching_items_left = stretching_items;
		unsigned stretching_size = space_left_for_stretching/stretching_items_left;
		for(splitter_item* item : items_)
		{
			if (item->is_visible() && item->geometry_ == geometry::stretching)
			{
				if (stretching_items_left == 1)
				{
					item->size_ = space_left_for_stretching;
				}
				else
				{
					item->size_ = stretching_size;
					space_left_for_stretching -= stretching_size;
				}
				stretching_items_left--;
			}
		}
	}

	apply_sizes<DIR>();
}

template<>
void splitter_section::apply_sizes<splitter::direction::horizontal>()
{
	unsigned left = position_.x;

	for(splitter_item* item : items_)
	{
		if (item->is_visible())
		{
			nct::position pos;
			pos.y = position_.y;
			pos.x = left;

			nct::size sz;
			sz.h = size_.h;
			sz.w = item->size_;

			item->apply_size(splitter::direction::vertical, pos, sz);

			left += item->size_ + 1;
		}
	}
}

template<>
void splitter_section::apply_sizes<splitter::direction::vertical>()
{
	unsigned top = position_.y;

	for(splitter_item* item : items_)
	{
		if (item->is_visible())
		{
			nct::position pos;
			pos.y = top;
			pos.x = position_.x;

			nct::size sz;
			sz.h = item->size_;
			sz.w = size_.w;

			item->apply_size(splitter::direction::horizontal, pos, sz);

			top += item->size_;
		}
	}
}

}
