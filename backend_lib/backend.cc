#include "backend.hh"

#include "event_dispatcher.hh"
#include "project.hh"
#include "messages.hh"

#include "utils_lib/log.hh"

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <cassert>
#include <stdexcept>

namespace cpped { namespace backend {

backend::~backend()
{
	if (pid_ != 0)
	{
		endpoint_.send_message(messages::stop{});
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
		// frontend process
		OPEN_LOG_FILE("cpped_front.log");
		LOG("frontend process started");

		::close(sockets[1]);
		endpoint_.set_fd(sockets[0]);
		return &endpoint_;
	}
	else
	{
		// backend (child) process
		OPEN_LOG_FILE("cpped_back.log");
		LOG("backend process started");

		::close(sockets[0]);
		endpoint_.set_fd(sockets[1]);

		event_dispatcher dispatcher(endpoint_);
		project pr(dispatcher);

		dispatcher.run();

		LOG("backend process finishing");
		return nullptr;
	}
}

}}
