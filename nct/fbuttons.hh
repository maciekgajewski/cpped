#pragma once

#include "event_window.hh"

#include <unordered_map>

namespace nct {

// Line with buttons associated with F keys. Think: Norton Commander, Midnight Commander, HTop
//
// There can be only one instance. Other widgets may control the text on the buttons using fbuttons_provider
class fbuttons : public event_window
{
public:

	fbuttons(event_dispatcher& ed, event_window* parent);
	~fbuttons();

	void update();
	bool try_special_key(int key_code);

private:

	void on_shown() override { update(); }

};


// Use this to provide text and action handlers for the buttons
class fbutton_action_provider
{
public:
	fbutton_action_provider();
	~fbutton_action_provider();

	void set_action(
		unsigned idx,
		const std::string& text,
		const std::function<void()>& handler);

	void clear_action(unsigned idx);

	bool has_action(unsigned idx) const
	{
		return actions_.find(idx) != actions_.end();
	}

	std::string get_text(unsigned idx) const
	{
		auto it = actions_.find(idx);
		assert(it != actions_.end());
		return it->second.text;
	}

	void call_handler(unsigned idx) const
	{
		auto it = actions_.find(idx);
		assert(it != actions_.end());
		if (it->second.handler)
		{
			it->second.handler();
		}
	}

private:

	struct action
	{
		std::string text;
		std::function<void()> handler;
	};

	std::unordered_map<unsigned, action> actions_;

};

}
