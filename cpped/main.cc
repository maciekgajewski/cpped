#include "editor_window.hh"
#include "styles.hh"
#include "project.hh"

#include "document_lib/document.hh"

#include "nct/ncurses_env.hh"
#include "nct/event_dispatcher.hh"

#include "backend_lib/backend.hh"

#include <boost/program_options.hpp>

#include <iostream>
#include <string>


using namespace std::literals::string_literals;

void usage(const boost::program_options::options_description& options)
{
	std::cout <<
R"(
CppEd - a C++ editor

usage: cpped [options] [file ...]                  edit specified file(s)
   or: cpped [options] --cmake <build directory>   open CMake project
)" << std::endl;

	options.print(std::cout);
}

void run_frontend(const boost::program_options::variables_map& options)
{
	cpped::project project;
	boost::optional<std::string> file_to_open;

	if (options.count("cmake"))
	{
		// TODO send request to backend
	}
	else
	{
		auto files = options["file"].as<std::vector<std::string>>();
		file_to_open = files[0];
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

int main(int argc, char** argv)
{
	namespace po = boost::program_options;

	po::options_description visible_options("Recognized options");
	visible_options.add_options()
		("help,h", "print this message")
		("cmake", po::value<std::string>(), "Open CMake project")
		;

	po::options_description hidden_options;
	hidden_options.add_options()
		("file", po::value<std::vector<std::string>>(), "input file")
		;

	po::options_description all_options;
	all_options.add(visible_options).add(hidden_options);

	po::positional_options_description positional_options;
	positional_options.add("file", -1);

	po::variables_map vm;

	try
	{
		po::store(po::command_line_parser(argc, argv)
			.options(all_options).positional(positional_options).run(), vm);
		po::notify(vm);
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		usage(visible_options);
		return 1;
	}

	if (vm.count("help"))
	{
		usage(visible_options);
		return 1;
	}

	cpped::backend::backend backend;
	cpped::backend::endpoint* endpoint = backend.fork();
	if (endpoint)
	{
		run_frontend(vm);
	}
}
