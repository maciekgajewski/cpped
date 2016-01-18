#include "background_worker_manager.hh"

#include "background_worker.hh"

#include <boost/range/algorithm.hpp>

namespace cpped { namespace backend {

namespace fs = boost::filesystem;

background_worker_manager::background_worker_manager()
{
	static const unsigned NUM_WORKERS = 2; // do we need to be smart about it?

	for(auto i = 0u; i < NUM_WORKERS; i++)
	{
		auto subprocess = std::make_unique<ipc::subprocess>(ipc::subprocess::fork(
			[i](ipc::endpoint& ep)
			{
				return background_worker_entry(i, ep);
			}));

		worker_t worker;
		worker.busy = false;
		worker.subprocess = std::move(subprocess);
		workers_.push_back(std::move(worker));
	}
}

void background_worker_manager::start()
{
	for(auto i = 0u; i < workers_.size(); i++)
	{
		worker_t& worker = workers_[i];
		worker.file_monitor = std::make_unique<utils::file_monitor>(
			worker.subprocess->get_endpoint().get_fd(),
			[sub = worker.subprocess.get()]()
			{
				sub->get_endpoint().receive_message();
			});

		worker.subprocess->get_endpoint().register_message_handler<worker_messages::parse_file_result>(
			[this, i](const worker_messages::parse_file_result& msg)
			{
				on_file_parsed(i, msg);
			});
	}
}

void background_worker_manager::parse_files(const std::vector<worker_messages::parse_file_request>& requests)
{
	for(const auto& request : requests)
	{
		file_queue_.push(request);
	}

	for(worker_t& worker: workers_)
	{
		if (!worker.busy)
		{
			send_next_job(worker);
		}
	}
}

void background_worker_manager::send_next_job(worker_t& worker)
{
	assert(!worker.busy);
	if (!file_queue_.empty())
	{
		worker_messages::parse_file_request req = std::move(file_queue_.front());
		file_queue_.pop();

		worker.subprocess->get_endpoint().send_message(req);
		worker.busy = true;
	}
}

void background_worker_manager::on_file_parsed(unsigned worker_idx, const worker_messages::parse_file_result& msg)
{
	worker_t& worker = workers_.at(worker_idx);
	assert(worker.busy);
	signal_file_parsed(msg);
	worker.busy = false;

	send_next_job(worker);
}

}}
