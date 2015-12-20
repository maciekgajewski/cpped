#include "backend.hh"

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <cassert>
#include <stdexcept>

namespace cpp { namespace backend {

backend::~backend()
{
	if (pid_ != 0)
	{
		// wait for child process
		::waitpid(pid_, nullptr, 0);
	}
}

endpoint* backend::fork()
{
	assert(pid_ == 0);

	int sockets[2];

	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
	{
		throw std::runtime_error("Unable to create sockets");
	}

	pid_ = ::fork();

	if (pid_ < 0)
	{
		throw std::runtime_error("fork failed");
	}

	if (pid_ != 0)
	{
		// we are in the parent process
		::close(sockets[1]);
		endpoint_.set_fd(sockets[0]);
		return &endpoint_;
	}
	else
	{
		::close(sockets[0]);
		endpoint_.set_fd(sockets[1]);
		// TODO run child here

		return nullptr;
	}
}

}}
