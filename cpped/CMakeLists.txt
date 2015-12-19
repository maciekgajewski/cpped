find_package(Boost REQUIRED COMPONENTS system filesystem)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_executable(cpped
	main.cc
	editor.cc editor.hh
	styles.hh styles.cc
	editor_window.cc editor_window.hh
	project.hh project.cc
	navigator_widget.cc navigator_widget.hh
)

target_link_libraries(cpped
	document_lib
	nct

	${Boost_LIBRARIES}
)
