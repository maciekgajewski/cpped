#pragma once

#include "buffered_writer.hh"
#include "socket_writer.hh"
#include "socket_reader.hh"
#include "serialize.hh"
#include "type_dispatcher.hh"

#include <cstddef>
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
	template<typename Msg>
	void send_message(const Msg& msg);

	// Registered handler will received decoded messages.
	// There can be at most one handler per type.
	template<typename Msg>
	void register_message_handler(const std::function<void(const Msg&)>& handler);

	// High-level message receiving.
	// Dispatcher is called with two params
	void receive_message();

private:

	void close();

	int fd_ = -1;
	type_dispatcher<socket_reader&> dispatcher_;
};

template<typename Msg>
void endpoint::send_message(const Msg& msg)
{
	socket_writer sw(fd_);
	buffered_writer<socket_writer> writer(sw);

	// This relay on the type-name strings having the same addresses in both processes
	type_id type = get_type_id<Msg>();

	serialize(writer, type);
	serialize(writer, msg);
}

template<typename Msg>
void endpoint::register_message_handler(const std::function<void(const Msg&)>& handler)
{
	dispatcher_.add_type<Msg>(
		[handler](socket_reader& reader)
		{
			Msg msg;
			deserialize(reader, msg);
			handler(msg);
		});
}

inline void endpoint::receive_message()
{
	socket_reader reader(fd_);
	type_id type;
	deserialize(reader, type);
	dispatcher_.dispatch(type, reader);
}

}}
