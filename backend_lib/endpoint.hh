#pragma once

#include <cstdlib>


namespace cpp { namespace backend {

// Communication endpoint used to talk to the other process
class endpoint
{
public:

	endpoint(int fd);
	endpoint(const endpoint&) = delete;
	endpoint(endpoint&& e);
	~endpoint();

	void set_fd(int fd);

	// High-level message sending, includes framing
	template<typename MSG>
	void send_message(std::uint32_t type, const MSG& msg);

private:

	void close();

	int fd_ = -1;
};

template<typename MSG>
void endpoint::send_message(std::uint32_t type, const MSG& msg)
{
	//buffered_writer writer
}

}}
