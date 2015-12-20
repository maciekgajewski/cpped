#pragma once

#include "socket_writer.hh"
#include "buffered_writer.hh"
#include "serialize.hh"

#include <boost/type_index.hpp>

#include <cstdlib>
#include <cstdint>

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
	void send_message(const MSG& msg);

private:

	void close();

	int fd_ = -1;
};

template<typename MSG>
void endpoint::send_message(const MSG& msg)
{
	socket_writer sw(fd_);
	buffered_writer<socket_writer> writer(sw);

	// This relay on the type-name strings having the same addresses in both processes
	std::intptr_t type = static_cast<std::intptr_t>(boost::typeindex::type_id<MSG>().raw_name());

	serialize(writer, type);
	serialize(writer, msg);
}

}}
