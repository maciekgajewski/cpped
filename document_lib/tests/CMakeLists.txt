add_executable(document_lib_tests
	document_tests.cc
)

target_link_libraries(document_lib_tests
	document_lib
	${Boost_unit_test_framework_LIBRARY}
)

