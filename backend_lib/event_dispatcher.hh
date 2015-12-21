#pragma once

#include "endpoint.hh"

namespace cpped { namespace backend {

// Baackend process' event loop and communication endpoint
class event_dispatcher
{
public:

	event_dispatcher(endpoint& ep);

	void run();

private:

	endpoint& endpoint_;
};

}}
