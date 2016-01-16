add_library(utils_lib
	log.cc log.hh
	event_loop.cc event_loop.hh
)

target_include_directories(utils_lib INTERFACE ..)


