#include "endpoint.hh"

#include <unistd.h>
#include <sys/select.h>

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
	::fd_set set;
	FD_ZERO(&set);
	FD_SET(fd_, &set);

	::timeval to{0, 0};

	int result = ::select(fd_+1, &set, nullptr, nullptr, &to);

	return result == 1;
}

void endpoint::close()
{
	if (fd_ >= 0)
	{
		::close(fd_);
	}
}

}}
