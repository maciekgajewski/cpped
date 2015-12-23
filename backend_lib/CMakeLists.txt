add_definitions(-DBOOST_LOG_DYN_LINK)

add_library(backend_lib
	backend.cc backend.hh
	endpoint.cc endpoint.hh
	socket_writer.cc socket_writer.hh
	serialize.hh
	buffered_writer.hh
	socket_reader.hh
	type_dispatcher.hh
	messages.hh
	event_dispatcher.cc event_dispatcher.hh
	project.cc project.hh
	cpp_tokens.cc cpp_tokens.hh
	project_connector.cc project_connector.hh
)

target_include_directories(backend_lib INTERFACE ..)

add_subdirectory(tests)

message("boost log setup=${Boost_LOG_SETUP_LIBRARY}")

target_link_libraries(backend_lib

	clang_lib

	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}

	pthread
)
