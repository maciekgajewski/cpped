find_package(Boost REQUIRED COMPONENTS system filesystem)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_executable(cpped
	main.cc
	ncurses_env.cc ncurses_env.hh
	ncurses_window.cc ncurses_window.hh
	editor.cc editor.hh
	styles.hh styles.cc
	editor_window.cc editor_window.hh
	project.hh project.cc
	event_dispatcher.cc event_dispatcher.hh
	event_window.cc event_window.hh
)

target_link_libraries(cpped
	document_lib

	ncursesw

	${Boost_LIBRARIES}
)
