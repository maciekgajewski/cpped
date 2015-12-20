#include "endpoint.hh"

#include <unistd.h>

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

}}
