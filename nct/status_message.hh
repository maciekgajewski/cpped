#pragma once

#include <string>
#include <functional>

namespace nct {

// Allows for setting status text displayed somwhere on the screen.
class status_provider
{
public:

	status_provider(const std::string& text = {});
	~status_provider();
	void set_status(const std::string& text);

private:
};

// recevies status messages set by providers
class status_message_receiver
{
public:

	template<typename Recevier>
	status_message_receiver(Recevier&& r)
		: recevier_(std::forward<Recevier>(r))
	{
		init();
	}

	~status_message_receiver();

private:

	void init();

	std::function<void(const std::string&)> recevier_;

};

}
