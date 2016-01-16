#pragma once

#include "ipc_lib/endpoint.hh"

#include <unistd.h>

namespace cpped { namespace backend {


// The Backend. Contains the backend process, takes care of forking and inter-process conmmunication
class backend
{
public:

	~backend();

	// Creates comm channel, forks process.
	// In child process - starts and run's event loop, returns nullptr at the end of the program.
	// In parent process - returns communication endpoint to talk to the child. Owns the endpoint.
	ipc::endpoint* fork();

private:

	pid_t pid_ = 0;
	ipc::endpoint endpoint_;
};

}}
