#pragma once

#include "ipc_lib/endpoint.hh"

#include <queue>

namespace cpped { namespace backend {

// Baackend process' event loop and communication endpoint
class event_dispatcher
{
public:

	event_dispatcher(ipc::endpoint& ep);

	template<typename Msg>
	void register_message_handler(const std::function<void(const Msg&)>& handler)
	{
		endpoint_.register_message_handler<Msg>(handler);
	}

	template<typename Msg>
	void send_message(const Msg& msg) { endpoint_.send_message(msg); }

	// the job will be executed when there is no messages from the frontend process
	void schedule_job(const std::function<void()>& job)
	{
		jobs_.push(job);
	}

	void run();

private:

	ipc::endpoint& endpoint_;
	std::queue<std::function<void()>> jobs_;
};

}}
