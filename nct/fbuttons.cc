#include "fbuttons.hh"

#include <cstdio>

namespace nct {

class fbuttons_action_stack
{
public:
	fbuttons_action_stack(fbuttons* w) : widget_(w) {}

	void add_provider(fbutton_action_provider* p)
	{
		providers_.push_back(p);
	}

	void remove_provider(fbutton_action_provider* p)
	{
		auto it = std::find(providers_.begin(), providers_.end(), p);
		providers_.erase(it);
	}

	void update() { widget_->update(); }

	std::string get_text(unsigned idx) const
	{
		auto it = std::find_if(
			providers_.begin(), providers_.end(),
			[&](const fbutton_action_provider* p) { return p->has_action(idx); });

		if (it == providers_.end())
		{
			return {};
		}
		else
		{
			return (*it)->get_text(idx);
		}
	}

	void call_handler(unsigned idx) const
	{
		auto it = std::find_if(
			providers_.begin(), providers_.end(),
			[&](const fbutton_action_provider* p) { return p->has_action(idx); });

		if (it != providers_.end())
		{
			(*it)->call_handler(idx);
		}
	}

private:
	fbuttons* widget_;
	std::vector<fbutton_action_provider*> providers_;
};

static const unsigned NBUTTONS = 10;
static fbuttons_action_stack* instance_ = nullptr;

fbuttons::fbuttons(event_dispatcher& ed, event_window* parent)
	: event_window(ed, parent)
{
	if (instance_)
		throw std::logic_error("There can be only one instance of fbuttons");
	instance_ = new fbuttons_action_stack(this);
}

fbuttons::~fbuttons()
{
	delete instance_;
	instance_ = nullptr;
}

void fbuttons::update()
{
	if (!is_visible()) return;
	ncurses_window& window = get_ncurses_window();

	style f_style = nct::style{COLOR_BLACK, COLOR_WHITE};
	style text_style = nct::style{COLOR_CYAN, COLOR_BLACK};

	char f_buf[8];

	window.style_fill_line(text_style, ' ', 0);
	for(unsigned i = 0; i < NBUTTONS; i++)
	{
		unsigned begin = std::floor(double(i) / NBUTTONS * window.get_width());
		unsigned end = std::floor(double(i+1) / NBUTTONS * window.get_width());

		int fl = std::snprintf(f_buf, 8, "F%d", i+1);
		window.move_cursor(0, begin);
		window.style_print(f_style, f_buf, fl);

		if (end > (begin+fl))
		{
			// TODO print text here
			window.style_print(text_style, instance_->get_text(i));
		}
	}
}

bool fbuttons::try_special_key(int key_code)
{
	if (key_code >= KEY_F(1) && key_code < KEY_F(1) + NBUTTONS)
	{
		int i = key_code - KEY_F(1);
		instance_->call_handler(i);
		return true;
	}

	return false;
}

fbutton_action_provider::fbutton_action_provider()
{
	if (!instance_)
		throw std::logic_error("fbutton_action_provider requires fbuttons widget");

	instance_->add_provider(this);
}

fbutton_action_provider::~fbutton_action_provider()
{
	assert(instance_);
	instance_->remove_provider(this);
}

void fbutton_action_provider::set_action(unsigned idx, const std::string& text, const std::function<void ()>& handler)
{
	assert(instance_);
	actions_[idx] = {text, handler};
	instance_->update();
}

void fbutton_action_provider::clear_action(unsigned idx)
{
	assert(instance_);
	actions_.erase(idx);
	instance_->update();
}

}
