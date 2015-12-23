#include "endpoint.hh"

#include <unistd.h>

#include <utility>

namespace cpped { namespace backend {

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

bool endpoint::has_message()
{
	// TODO
	return true;
}

void endpoint::close()
{
	if (fd_ >= 0)
	{
		::close(fd_);
	}
}

}}
