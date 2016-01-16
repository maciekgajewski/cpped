#include "subprocess.hh"

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#include <cassert>
#include <stdexcept>

namespace cpped { namespace ipc {

subprocess::~subprocess()
{
	if (pid_ != 0)
	{
		::kill(pid_, SIGKILL);
		::waitpid(pid_, nullptr, 0);
	}
}

ipc::endpoint& subprocess::fork(const subprocess_entry& child_process_entry)
{
	if (pid_ != 0)
		throw std::logic_error("Process already started");

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
		// parent process
		::close(sockets[1]);
		endpoint_.set_fd(sockets[0]);
		return endpoint_;
	}
	else
	{
		// child process
		::close(sockets[0]);
		endpoint_.set_fd(sockets[1]);

		int return_code = child_process_entry(endpoint_);
		std::exit(return_code);
	}
}

}}
