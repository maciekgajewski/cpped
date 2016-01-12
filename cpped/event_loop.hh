#pragma once

#include <functional>
#include <memory>

namespace cpped {

// Simple event loop, observing number if file descriptors for readability
class event_loop
{
public:

	event_loop();
	~event_loop();

	static event_loop* get_current();

	// starts the event loop. Will return only after stop is called
	void run();

	// interrupts event loop
	void stop() { run_ = false; }

private:
	bool run_;
};

// File registration. When createdm will register file in event loop and cal handler whenever file becomes readable
class observed_file
{
public:
	using handler_t = std::function<void()>;

	observed_file(int fd, const handler_t& handler);
	~observed_file();

private:

	int fd_;
	handler_t handler_;

	friend class event_loop;
};

}
