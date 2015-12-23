#include "backend.hh"

#include "event_dispatcher.hh"
#include "project.hh"
#include "messages.hh"

#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/logger.hpp>

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
	namespace bl = boost::log;

	auto oct = get_type_id<messages::open_cmake_project>();
	auto st = get_type_id<messages::stop>();

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
		bl::add_file_log(
			bl::keywords::file_name = "cpped_front.log",
			bl::keywords::format = "[%TimeStamp%]: %Message%");
		bl::add_common_attributes();

		bl::sources::logger logger;

		BOOST_LOG(logger) << "Frontend process started";

		::close(sockets[1]);
		endpoint_.set_fd(sockets[0]);
		return &endpoint_;
	}
	else
	{
		// child process


		bl::add_file_log(
			bl::keywords::file_name = "cpped_back.log",
			bl::keywords::format = "[%TimeStamp%]: %Message%");
		bl::add_common_attributes();

		bl::sources::logger logger;

		BOOST_LOG(logger) << "Backend process started";

		::close(sockets[0]);
		endpoint_.set_fd(sockets[1]);

		event_dispatcher dispatcher(endpoint_);
		//dispatcher.run();

		return nullptr;
	}
}

}}
