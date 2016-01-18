#pragma once

#include "background_worker_messages.hh"

#include "ipc_lib/subprocess.hh"

#include "utils_lib/event_loop.hh"

#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

#include <vector>
#include <queue>

namespace cpped { namespace backend {

// Manages worker processes
class background_worker_manager
{
public:
	// starts the worker processes
	background_worker_manager();

	// Call this one the event_loop is created, to register sockets
	void start();

	// job intake
	void parse_files(const std::vector<worker_messages::parse_file_request>& files);

	// result output
	boost::signals2::signal<void(const worker_messages::parse_file_result&)> signal_file_parsed;

private:

	struct worker_t
	{
		bool busy = false;
		std::unique_ptr<ipc::subprocess> subprocess;
		std::unique_ptr<utils::file_monitor> file_monitor;
	};

	void send_next_job(worker_t& worker);
	void on_file_parsed(unsigned worker_idx, const worker_messages::parse_file_result& msg);

	std::queue<worker_messages::parse_file_request> file_queue_;
	std::vector<worker_t> workers_;
};

}}
