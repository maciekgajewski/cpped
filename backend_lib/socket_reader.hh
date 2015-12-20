#pragma once

#include <cstdlib>

#include <unistd.h>

namespace cpp { namespace backend {

class socket_reader
{
public:
	socket_reader(int fd) : fd_(fd) {}
	std::size_t read(const void* data, std::size_t sz)
	{
		return ::read(fd_, data, sz);
	}

private:

	int fd_;
};
