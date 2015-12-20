add_library(backend_lib
	backend.cc backend.hh
	endpoint.cc endpoint.hh
)

target_include_directories(backend_lib INTERFACE ..)

target_link_libraries(backend_lib

	clang_lib

	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
)
