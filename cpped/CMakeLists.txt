add_executable(cpped
	main.cc
	editor.cc editor.hh
	styles.hh styles.cc
	editor_window.cc editor_window.hh
	project.hh project.cc
	main_window.cc main_window.hh
	completer_widget.cc completer_widget.hh
	clipboard.cc clipboard.hh
	edited_file.cc edited_file.hh
	open_files_widget.cc open_files_widget.hh
	editor_state.hh
	command_widget.cc command_widget.hh
	commands.cc commands.hh
)

target_link_libraries(cpped
	document_lib
	nct
	backend_lib
	utils_lib
	ipc_lib

	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)
