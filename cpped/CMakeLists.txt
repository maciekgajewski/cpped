add_executable(cpped
	main.cc
	ncurses_env.cc ncurses_env.hh
	ncurses_window.cc ncurses_window.hh
	editor.cc editor.hh
	styles.hh styles.cc
	editor_window.cc editor_window.hh
)

target_link_libraries(cpped
	document_lib

	ncursesw
)
