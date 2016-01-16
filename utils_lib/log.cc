#include "log.hh"

#ifdef ENABLE_LOGGING

namespace cpped { namespace logging {

std::ofstream* logger = nullptr;

void open_log_file(const std::string& path)
{
	if (logger)
		delete logger;

	logger = new std::ofstream(path, std::ios_base::out | std::ios_base::trunc);
}

}}

#endif
