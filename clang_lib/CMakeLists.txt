add_library(clang_lib
	clang.hh clang.cc
	flags.cc flags.hh
	utils.cc utils.hh
)

target_include_directories(clang_lib INTERFACE ..)

target_link_libraries(clang_lib

	${Boost_FILESESTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${CLANG_LIBRARIES}
)
