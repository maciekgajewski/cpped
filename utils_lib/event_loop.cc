#include "event_loop.hh"

#include "log.hh"

#include <boost/range/algorithm.hpp>

#include <stdexcept>
#include <cassert>
#include <vector>
#include <chrono>

#include <sys/epoll.h>
#include <unistd.h>
#include <signal.h>

using namespace  std::literals::chrono_literals;

namespace cpped { namespace utils {

class epoll_observer
{
public:

	epoll_observer()
	{
		epoll_fd_ = ::epoll_create1(0);
		if (epoll_fd_ < 0)
			throw std::runtime_error("Failed ot create epoll");
	}

	~epoll_observer()
	{
		::close(epoll_fd_);
	}

	void observe_for_readablity(int fd, epoll_data_t data)
	{
		epoll_event event;
		event.data = data;
		event.events = EPOLLIN;

		int r = ::epoll_ctl(
			epoll_fd_,
			EPOLL_CTL_ADD,
			fd,
			&event);

		if (r < 0)
			throw std::runtime_error("Error adding file to epoll");

		files_++;
	}

	void remove(int fd)
	{
	int r = ::epoll_ctl(
			epoll_fd_,
			EPOLL_CTL_DEL,
			fd,
			nullptr);

		if (r < 0)
			throw std::runtime_error("Error removing file to epoll");
		assert(files_ > 0);
		files_--;
	}

	void wait(std::chrono::duration<double> timeout, std::vector<epoll_event>& events)
	{
		events.resize(files_);
		sigset_t mask;
		::sigemptyset(&mask);
		::sigaddset(&mask, SIGWINCH);

		int r = ::epoll_pwait(
			epoll_fd_,
			events.data(),
			files_,
			static_cast<int>(timeout/1ms),
			&mask);

		// TODO need to handle SIGWINCH gracefully, instead of ignoring it
		if (r < 0)
			throw std::runtime_error("epoll_wait failed");

		assert(r <= files_);
		events.resize(r);
	}

private:

	int epoll_fd_;
	unsigned files_ = 0;
};

static thread_local epoll_observer* current_observer = nullptr;
static thread_local event_loop* current_instance = nullptr;

event_loop::event_loop()
{
	LOG("Creating event_loop, this=" << this << ", current_instance=" << current_instance);
	if (current_instance)
		throw std::logic_error("Only onve instance of event_loop per thread allowed");
	assert(current_observer == nullptr);
	current_instance = this;
	current_observer = new epoll_observer;
}

event_loop::~event_loop()
{
	delete current_observer;
	current_observer = nullptr;
	current_instance = nullptr;
}

event_loop* event_loop::get_current()
{
	return current_instance;
}

void event_loop::run()
{
	assert(current_instance);
	run_ = true;

	while(run_)
	{
		std::vector<epoll_event> events;
		current_observer->wait(10ms, events);

		if(events.empty())
		{
			on_idle();
		}
		else for(const epoll_event& event : events)
		{
			file_monitor* f = static_cast<file_monitor*>(event.data.ptr);
			f->handler_();
		}
	}
}

void event_loop::on_idle()
{
	for(const auto& pair : idle_handlers_)
	{
		pair.second();
	}
}

std::uint64_t event_loop::add_idle_handler(const std::function<void ()>& h)
{
	static std::uint64_t next_id = 0;
	auto id = next_id++;

	idle_handlers_.emplace_back(id, h);
	return id;
}

void event_loop::remove_idle_handler(std::uint64_t id)
{
	auto it = boost::find_if(idle_handlers_,
		[&](const auto& pair) { return pair.first == id; });
	assert(it != idle_handlers_.end());
	idle_handlers_.erase(it);
}

file_monitor::file_monitor(int fd, const file_monitor::handler_t& handler)
	: fd_(fd), handler_(handler)
{
	assert(handler_);
	if (!current_observer)
		throw std::logic_error("Event loop not availabale");

	epoll_data_t data;
	data.ptr = this;
	current_observer->observe_for_readablity(fd, data);
}

file_monitor::~file_monitor()
{
	assert(current_observer);
	current_observer->remove(fd_);
}

idle_monitor::idle_monitor(const idle_monitor::handler_t& handler)
{
	assert(handler);
	if (!current_instance)
		throw std::logic_error("Event loop not availabale");

	id_ = current_instance->add_idle_handler(handler);
}

idle_monitor::~idle_monitor()
{
	assert(current_instance);
	current_instance->remove_idle_handler(id_);
}

}}
