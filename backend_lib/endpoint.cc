#include "endpoint.hh"

#include <unistd.h>

#include <stdexcept>
#include <utility>

namespace cpp { namespace backend {

endpoint::endpoint(int fd)
	: fd_(fd)
{
}

endpoint::endpoint(endpoint&& e)
{
	std::swap(fd_, e.fd_);
}

endpoint::~endpoint()
{
	close();
}

void endpoint::set_fd(int fd)
{
	close();
	fd_ = fd;
}

void endpoint::close()
{
	if (fd_ >= 0)
	{
		::close(fd_);
	}
}

void endpoint::send(const char* data, std::size_t sz)
{
	std::size_t sent = 0;
	while(sent < sz)
	{
		auto res = ::write(fd_, data + sent, sz-sent);
		if (res <= 0)
		{
			throw std::runtime_error("Error writing to socket");
		}
		sent += res;
	}
}

}}
