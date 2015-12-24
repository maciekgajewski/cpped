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
	log.cc log.hh
	compilation_unit.hh compilation_unit.cc
)

target_include_directories(backend_lib INTERFACE ..)

add_subdirectory(tests)

target_link_libraries(backend_lib

	clang_lib

	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_DATE_TIME_LIBRARY}
)
