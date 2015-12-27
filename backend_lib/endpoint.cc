#include "endpoint.hh"

#include <unistd.h>
#include <sys/select.h>

#include <utility>
#include <chrono>

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

static bool select_socket(int fd, ::timeval* to)
{
	::fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);

	int result = ::select(fd+1, &set, nullptr, nullptr, to);

	return result == 1;
}

bool endpoint::has_message()
{
	::timeval to{0, 0};
	return select_socket(fd_, &to);
}

bool endpoint::wait_for_message(std::chrono::duration<double> timeout)
{
	using namespace  std::literals::chrono_literals;
	int seconds = int(std::floor(timeout/1s));
	int useconds = int((timeout - seconds*1s)/1us);

	::timeval to{seconds, useconds};
	return select_socket(fd_, &to);
}

void endpoint::close()
{
	if (fd_ >= 0)
	{
		::close(fd_);
	}
}

}}
