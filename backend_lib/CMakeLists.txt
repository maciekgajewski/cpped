find_package(Boost REQUIRED COMPONENTS system filesystem)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_library(backend_lib
	backend.cc backend.hh
	endpoint.cc endpoint.hh
)

target_include_directories(backend_lib INTERFACE ..)

target_link_libraries(backend_lib

	clang_lib

	${Boost_LIBRARIES}

)
