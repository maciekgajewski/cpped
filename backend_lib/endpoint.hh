#pragma once

#include "socket_writer.hh"
#include "socket_reader.hh"
#include "buffered_writer.hh"
#include "serialize.hh"
#include "type_dispatcher.hh"

//#include <boost/type_index.hpp>

#include <cstdlib>
#include <cstdint>

namespace cpp { namespace backend {


class deserializing_dispatcher
{
public:

	template<typename Msg>
	void add_type()
	{
		dispatcher_.add_type(
			[](socket_reader& reader)
			{
				Msg msg;
				deserialize(reader, msg);

			});
	}

	void dispatch(type_id type)
	{
		// TODO
	};

private:

	type_dispatcher<socket_reader& reader> dispatcher_;
};

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

	// High-level message receiving.
	// Dispatcher is called with two params
	template<typename TypeDispatcher>
	void receive_message(const TypeDispatcher& d);

private:

	void close();

	int fd_ = -1;
};

template<typename Msg>
void endpoint::send_message(const Msg& msg)
{
	socket_writer sw(fd_);
	buffered_writer<socket_writer> writer(sw);

	// This relay on the type-name strings having the same addresses in both processes
	type_id = get_type_id<Msg>();

	serialize(writer, type);
	serialize(writer, msg);
}

void endpoint::receive_message()
{
}

}}
