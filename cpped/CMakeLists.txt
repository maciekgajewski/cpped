add_executable(cpped
	main.cc
	ncurses_env.cc ncurses_env.hh
	ncurses_window.cc ncurses_window.hh
	editor.cc editor.hh
	styles.hh styles.cc
)

target_link_libraries(cpped
	document_lib

	ncurses

)
