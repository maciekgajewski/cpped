#include "event_dispatcher.hh"

#include "messages.hh"

namespace cpped { namespace backend {

event_dispatcher::event_dispatcher(endpoint& ep)
	: endpoint_(ep)
{
}

void event_dispatcher::run()
{
	using namespace  std::literals::chrono_literals;

	bool run = true;
	endpoint_.register_message_handler<messages::stop>(
		[&](const messages::stop&) { run = false; });

	while(run)
	{
		if (jobs_.empty())
		{
			// block until messages
			endpoint_.receive_message();
		}
		else
		{
			if (endpoint_.wait_for_message(1s))
			{
				endpoint_.receive_message();
			}
			else
			{
				auto job = jobs_.front();
				jobs_.pop();
				job();
			}
		}
	}
}

}}
