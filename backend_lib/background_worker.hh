#pragma once

#include "ipc_lib/endpoint.hh"

namespace cpped { namespace backend {

int background_worker_entry(unsigned worker_number, ipc::endpoint& ep);

}}
