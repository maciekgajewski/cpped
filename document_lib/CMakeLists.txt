add_library(document_lib
	document.hh document.cc
	iparser.hh
	cpp_parser.cc cpp_parser.hh
	document_data.cc document_data.hh
	line_data.cc line_data.hh
	line_token.hh line_token.cc
)

target_include_directories(document_lib INTERFACE ..)

add_subdirectory(tests)

target_link_libraries(document_lib

	clang_lib

	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
)
