#include "editor_window.hh"
#include "styles.hh"
#include "project.hh"

#include "document_lib/document.hh"
#include "document_lib/cpp_parser.hh"

#include "nct/ncurses_env.hh"
#include "nct/event_dispatcher.hh"

#include <iostream>
#include <string>


using namespace std::literals::string_literals;

// zażółć gęślą jaźń
//		t
//x		t
//xx	t
//xxx	t
//xxxx	t
//xxxxx	t
int main(int argc, char** argv)
{
	cpped::document::document document;
	cpped::project project;
	if (argc > 1)
	{
		if (argv[1] == "-cmake"s)
		{
			if (argc < 3)
			{
				throw std::runtime_error("-cmake requires cmake build dir as an argument");
			}
			project = cpped::load_cmake_project(argv[2]);
		}
		else
		{
			document.load_from_file(argv[1], std::make_unique<cpped::document::cpp_parser>());
			document.parse_language();
		}
	}

	::setlocale(LC_ALL, "en_EN.utf-8");
	nct::ncurses_env env;
	auto scr = env.get_stdscr();

	nct::event_dispatcher dispatcher;
	cpped::style_manager styles(dispatcher.get_palette());
	cpped::editor_window editor(dispatcher, styles, document);
	editor.set_size(scr.get_size());
	editor.set_active(); // so it recevies input
	editor.show();

	dispatcher.set_global_quit_key("^X");
	dispatcher.run();
}
