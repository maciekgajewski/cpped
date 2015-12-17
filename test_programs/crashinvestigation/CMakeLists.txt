add_executable(crash
	crash.cc
)

target_link_libraries(crash
	ncursesw
	${CLANG_LIBRARIES}
)

