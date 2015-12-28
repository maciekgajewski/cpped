add_executable(cpped
	main.cc
	editor.cc editor.hh
	styles.hh styles.cc
	editor_window.cc editor_window.hh
	project.hh project.cc
	navigator_widget.cc navigator_widget.hh
	main_window.cc main_window.hh
)

target_link_libraries(cpped
	document_lib
	nct
	backend_lib

	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)
