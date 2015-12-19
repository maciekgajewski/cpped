find_package(Boost REQUIRED COMPONENTS system filesystem)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_library(clang_lib
	clang.hh clang.cc
	clang_flags.cc clang_flags.hh
)

target_include_directories(clang_lib INTERFACE ..)

target_link_libraries(clang_lib

	${Boost_LIBRARIES}
	${CLANG_LIBRARIES}

)
