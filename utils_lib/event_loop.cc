#include "event_loop.hh"

#include <stdexcept>
#include <cassert>
#include <vector>

#include <sys/epoll.h>
#include <unistd.h>

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

	void wait(int ms, std::vector<epoll_event>& events)
	{
		events.resize(files_);

		int r = ::epoll_wait(
			epoll_fd_,
			events.data(),
			files_,
			ms);

		if (r < 0)
			throw std::runtime_error("epoll_wait failed");

		assert(r < files_);
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
		current_observer->wait(10 /*ms*/, events);

		for(const epoll_event& event : events)
		{
			observed_file* f = static_cast<observed_file*>(event.data.ptr);
			f->handler_();
		}
	}
}

observed_file::observed_file(int fd, const observed_file::handler_t& handler)
	: fd_(fd), handler_(handler)
{
	assert(handler_);
	if (!current_observer)
		throw std::logic_error("Event loop not availabale");

	epoll_data_t data;
	data.ptr = this;
	current_observer->observe_for_readablity(fd, data);
}

observed_file::~observed_file()
{
	assert(current_observer);
	current_observer->remove(fd_);
}

}}
