#pragma once

#include "endpoint.hh"

#include <functional>

#include <unistd.h>

namespace cpped { namespace ipc {

// Represends subprocess. Takes care of forkoing and creating inter-process channel
class subprocess
{
public:

	using subprocess_entry = std::function<int(ipc::endpoint& ep)>;

	// If subprocess started, kills it
	virtual ~subprocess();

	// Starts the subprocess, returns communication endpoint
	ipc::endpoint& fork(const subprocess_entry& child_process_entry);

private:

	pid_t pid_ = 0;
	ipc::endpoint endpoint_;
};

}}
