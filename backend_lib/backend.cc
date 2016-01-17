#include "backend.hh"

#include "event_dispatcher.hh"
#include "project.hh"

#include "utils_lib/log.hh"
#include "utils_lib/event_loop.hh"

namespace cpped { namespace backend {


int run_backend_process(ipc::endpoint& ep)
{
	background_worker_manager bwm;

	OPEN_LOG_FILE("cpped_back.log");
	LOG("backend process started");


	event_dispatcher dispatcher(ep);
	bwm.start();
	project pr(dispatcher, bwm);

	dispatcher.run();

	LOG("backend process finishing");
	return 0;
}

}}
