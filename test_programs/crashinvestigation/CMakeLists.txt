add_executable(crash
	crash.cc
)

target_link_libraries(crash
	ncursesw
	LLVM-3.7
	clang-3.7

)

