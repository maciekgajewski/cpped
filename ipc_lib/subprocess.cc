#include "subprocess.hh"

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#include <cassert>
#include <stdexcept>

namespace cpped { namespace ipc {

subprocess::subprocess(subprocess&& o)
	: endpoint_(std::move(o.endpoint_))
{
	std::swap(o.pid_, pid_);
}

subprocess::~subprocess()
{
	if (pid_ != 0)
	{
		::kill(pid_, SIGKILL);
		::waitpid(pid_, nullptr, 0);
	}
}

subprocess subprocess::fork(const subprocess_entry& child_process_entry)
{
	int sockets[2];

	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
	{
		throw std::runtime_error("Unable to create sockets");
	}

	pid_t pid = ::fork();

	if (pid < 0)
	{
		throw std::runtime_error("fork failed");
	}

	endpoint ep;

	if (pid != 0)
	{
		// parent process
		::close(sockets[1]);
		ep.set_fd(sockets[0]);
		return subprocess(pid, std::move(ep));
	}
	else
	{
		// child process
		::close(sockets[0]);
		ep.set_fd(sockets[1]);

		int return_code = child_process_entry(ep);
		std::exit(return_code);
	}
}

subprocess::subprocess(pid_t pid, endpoint&& ep)
	: pid_(pid), endpoint_(std::move(ep))
{

}

}}
