#include "socket_writer.hh"

#include <unistd.h>

#include <stdexcept>

namespace cpped { namespace backend {


void socket_writer::write(const void* data, std::size_t sz)
{
	std::size_t sent = 0;
	while(sent < sz)
	{
		auto res = ::write(fd_, reinterpret_cast<const char*>(data) + sent, sz-sent);
		if (res <= 0)
		{
			throw std::runtime_error("Error writing to socket");
		}
		sent += res;
	}
}

}}
