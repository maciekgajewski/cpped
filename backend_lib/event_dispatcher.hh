#pragma once

#include "endpoint.hh"

#include <queue>

namespace cpped { namespace backend {

// Baackend process' event loop and communication endpoint
class event_dispatcher
{
public:

	event_dispatcher(endpoint& ep);

	template<typename Msg>
	void register_message_handler(const std::function<void(const Msg&)>& handler)
	{
		endpoint_.register_message_handler<Msg>(handler);
	}

	// the job will be executed when there is no messages from the frontend process
	void schedule_job(const std::function<void()>& job)
	{
		jobs_.push(job);
	}

	void run();

private:

	endpoint& endpoint_;
	std::queue<std::function<void()>> jobs_;


};

}}
