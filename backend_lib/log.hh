#pragma once

#ifdef ENABLE_LOGGING

#include <fstream>

#include <boost/date_time.hpp>

namespace cpped { namespace logging {

std::ofstream* logger = nullptr;

inline void open_log_file(const std::string& path)
{
	if (logger)
		delete logger;

	logger = new std::ofstream(path, std::ios_base::out | std::ios_base::trunc);

	//boost::date_time::local_
}

#define OPEN_LOG_FILE(filename) ::cpped::logging::open_log_file(filename)
#define LOG(whatever) do { if(::cpped::logging::logger) { \
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time(); \
	*::cpped::logging::logger << boost::posix_time::to_simple_string(now) << " " << whatever << std::endl; }} while(false)

}}

#else

#define OPEN_LOG_FILE(filename)
#define LOG(param)

#endif
