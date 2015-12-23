#pragma once

#include "event_dispatcher.hh"
#include "project.hh"

namespace cpped { namespace backend {

// A connector between backend::project and the IPC system
class project_connector
{
public:

	project_connector(project& pr, event_dispatcher& ed);
};

}}
