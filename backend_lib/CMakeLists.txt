add_library(backend_lib
	backend.cc backend.hh
	messages.hh
	event_dispatcher.cc event_dispatcher.hh
	project.cc project.hh
	cpp_tokens.cc cpp_tokens.hh
	compilation_unit.hh compilation_unit.cc
	open_file.cc open_file.hh
	completion.cc completion.hh
	background_worker.cc background_worker.hh
	background_worker_messages.hh
	background_worker_manager.cc background_worker_manager.hh
)

target_include_directories(backend_lib INTERFACE ..)

add_subdirectory(tests)

target_link_libraries(backend_lib

	clang_lib
	utils_lib
	ipc_lib
	document_lib

	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_DATE_TIME_LIBRARY}
)
