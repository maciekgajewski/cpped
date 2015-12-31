#include "status_message.hh"

#include <vector>
#include <algorithm>
#include <cassert>

namespace nct {

class status_message_stack
{
public:

	status_message_stack(const std::function<void(const std::string&)>& r)
		: recevier_(r)
	{
	}

	void push(status_provider* provider, const std::string& text)
	{
		providers_.push_back({provider, text});
		display_message(text);
	}

	void pop(status_provider* provider)
	{
		auto it = std::find_if(
			providers_.begin(), providers_.end(),
			[&](const auto& pair) { return pair.first == provider; });
		assert(it != providers_.end());

		providers_.erase(it);

		if (providers_.empty())
		{
			display_message({});
		}
		else
		{
			display_message(providers_.back().second);
		}
	}

	void set_message(status_provider* provider, const std::string& msg)
	{
		auto it = std::find_if(
			providers_.begin(), providers_.end(),
			[&](const auto& pair) { return pair.first == provider; });

		it->second = msg;
		if (it == providers_.end()-1)
		{
			display_message(msg);
		}

	}

private:

	void display_message(const std::string& msg)
	{
		if (recevier_)
			recevier_(msg);
	}

	const std::function<void(const std::string&)>& recevier_;
	std::vector<std::pair<status_provider*, std::string>> providers_;
};

static status_message_stack* message_stack = nullptr;

status_provider::status_provider(const std::string& text)
{
	if (message_stack)
		message_stack->push(this, text);
}

status_provider::~status_provider()
{
	if (message_stack)
		message_stack->pop(this);
}

void status_provider::set_status(const std::__cxx11::string& text)
{
	if (message_stack)
		message_stack->set_message(this, text);
}

status_message_receiver::~status_message_receiver()
{
	delete message_stack;
	message_stack = nullptr;
}

void status_message_receiver::init()
{
	// TODO this is oversimplified, it only allows for one status_message_receiver.
	// they shouild be stacked as well
	if (!message_stack)
		message_stack = new status_message_stack(recevier_);
}

}
