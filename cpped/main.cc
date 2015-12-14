#include "editor_window.hh"
#include "styles.hh"
#include "project.hh"

#include "document_lib/document.hh"

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
	cpped::project project;
	boost::optional<std::string> file_to_open;
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
			file_to_open = argv[1];
		}
	}

	::setlocale(LC_ALL, "en_EN.utf-8");
	nct::ncurses_env env;
	auto scr = env.get_stdscr();

	nct::event_dispatcher dispatcher;
	cpped::style_manager styles(dispatcher.get_palette());
	cpped::editor_window editor(project, dispatcher, styles);

	if (file_to_open)
	{
		editor.open_file(*file_to_open);
	}

	editor.set_size(scr.get_size());
	editor.set_active(); // so it recevies input
	editor.show();

	dispatcher.set_global_quit_key("^X");
	dispatcher.run();
}
