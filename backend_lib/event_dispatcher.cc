#include "event_dispatcher.hh"

#include "messages.hh"

namespace cpped { namespace backend {

event_dispatcher::event_dispatcher(ipc::endpoint& ep)
	: endpoint_(ep)
{
}

void event_dispatcher::run()
{
	endpoint_.register_message_handler<messages::stop>(
		[&](const messages::stop&) { loop_.stop(); });

	utils::file_monitor endpoint_monitor(endpoint_.get_fd(),
		[&]()
		{
			endpoint_.receive_message();
		});

	utils::idle_monitor idle_monitor(
		[&]()
		{
			if (!jobs_.empty())
			{
				auto job = jobs_.front();
				jobs_.pop();
				job();
			}
		});

	loop_.run();
}

}}
