#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <cstdint>

namespace cpped { namespace utils {

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

	using idle_registration = std::pair<std::uint64_t, std::function<void()>>;
	friend class idle_monitor;

	void on_idle();
	std::uint64_t add_idle_handler(const std::function<void()>& h);
	void remove_idle_handler(std::uint64_t id);

	bool run_;
	std::vector<idle_registration> idle_handlers_;

};

// File registration. When createdm will register file in event loop and cal handler whenever file becomes readable
class file_monitor
{
public:
	using handler_t = std::function<void()>;

	file_monitor(int fd, const handler_t& handler);
	~file_monitor();

private:

	int fd_;
	handler_t handler_;

	friend class event_loop;
};

// On-idle registration. Notified when there is no incoming events
class idle_monitor
{
public:
	using handler_t = std::function<void()>;

	idle_monitor(const handler_t& handler);
	~idle_monitor();

private:

	std::uint64_t id_;
};

}}
