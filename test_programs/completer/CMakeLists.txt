add_executable(completer
	completer.cc
)

target_link_libraries(completer
	document_lib
	clang_lib
)

