add_library(nct
	window_manager.cc window_manager.hh
	event_window.cc event_window.hh
	ncurses_env.cc ncurses_env.hh
	ncurses_window.cc ncurses_window.hh
	types.hh
	ncurses_inc.hh
	line_edit.cc line_edit.hh
	list_widget.cc list_widget.hh
	status_message.cc status_message.hh
	style.cc style.hh
	fbuttons.cc fbuttons.hh
	splitter.cc splitter.hh
)

target_include_directories(nct INTERFACE ..)

target_link_libraries(nct

	ncursesw
	utils_lib
)
