add_library(document_lib
	document.hh document.cc
	clang.hh clang.cc
	iparser.hh
	cpp_parser.cc cpp_parser.hh
)

target_include_directories(document_lib INTERFACE ..)

add_subdirectory(tests)

target_link_libraries(document_lib

	LLVM-3.7
	clang-3.7
)
