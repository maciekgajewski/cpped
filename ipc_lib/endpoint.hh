#pragma once

#include "buffered_writer.hh"
#include "socket_writer.hh"
#include "socket_reader.hh"
#include "serialize.hh"
#include "type_dispatcher.hh"

#include "utils_lib/log.hh"

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <chrono>

namespace cpped { namespace ipc {

// Communication endpoint used to talk to the other process
class endpoint
{
public:

	endpoint() = default;
	endpoint(int fd);
	endpoint(const endpoint&) = delete;
	endpoint(endpoint&& e);
	~endpoint();

	void set_fd(int fd);
	int get_fd() const { return fd_; }

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

	// Checks if there is a massage awaiting.
	// If returns 'false', the next call to receive_message() is likely to block
	bool has_message();

	// Waits for message. Returns 'false' if no message recevied in specified period
	bool wait_for_message(std::chrono::duration<double> timeout);

	// Sends the request, and spins waiting for reply
	template<typename Request, typename Reply>
	void send_sync_request(const Request& request, Reply& reply);

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

	LOG("Sending message, type=" << type);

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

	LOG("Received message, type=" << type);

	dispatcher_.dispatch(type, reader);
}

template<typename Request, typename Reply>
void endpoint::send_sync_request(const Request& request, Reply& reply)
{
	send_message(request);
	type_id reply_type = get_type_id<Reply>();

	while(true) // TODO needs timeout?
	{
		socket_reader reader(fd_);
		type_id type;
		deserialize(reader, type);

		LOG("Received message, type=" << type << ", waiting for type=" << reply_type);

		if (type == reply_type)
		{
			deserialize(reader, reply);
			return;
		}

		dispatcher_.dispatch(type, reader);
	}
}

}}
