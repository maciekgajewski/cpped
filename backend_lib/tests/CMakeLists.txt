add_executable(backend_lib_tests
	main.cc

	cppparser_tests.cc
)

target_link_libraries(backend_lib_tests
	backend_lib
	document_lib
)

add_test(backend_lib_tests backend_lib_tests)
