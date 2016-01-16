#pragma once

#include <cstdlib>

#include <unistd.h>

namespace cpped { namespace ipc {

class socket_writer
{
public:
	socket_writer(int fd) : fd_(fd) {}
	void write(const void* data, std::size_t sz);

private:

	int fd_;
};

}}
