add_library(ipc_lib
	buffered_writer.hh
	endpoint.cc
	endpoint.hh
	serialize.hh
	socket_reader.hh
	socket_writer.cc
	socket_writer.hh
	type_dispatcher.hh
)

target_include_directories(ipc_lib INTERFACE ..)

target_link_libraries(ipc_lib
	utils_lib
)
