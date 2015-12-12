add_library(nct
	event_dispatcher.cc event_dispatcher.hh
	event_window.cc event_window.hh
	ncurses_env.cc ncurses_env.hh
	ncurses_window.cc ncurses_window.hh
	types.hh
	ncurses_inc.hh
)

target_include_directories(nct INTERFACE ..)

target_link_libraries(nct

	ncursesw
)
