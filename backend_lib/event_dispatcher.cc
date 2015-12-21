#include "event_dispatcher.hh"

#include "messages.hh"

namespace cpped { namespace backend {

event_dispatcher::event_dispatcher(endpoint& ep)
	: endpoint_(ep)
{
}

void event_dispatcher::run()
{
	bool run = true;
	endpoint_.register_message_handler<messages::stop>(
		[&](const messages::stop&) { run = false; });

	while(run)
	{
		endpoint_.receive_message();
	}
}

}}
