#include "editor_window.hh"
#include "main_window.hh"
#include "styles.hh"
#include "project.hh"
#include "clipboard.hh"

#include "utils_lib/event_loop.hh"

#include "document_lib/document.hh"

#include "nct/ncurses_env.hh"
#include "nct/window_manager.hh"

#include "backend_lib/backend.hh"

#include "ipc_lib/subprocess.hh"

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

void run_frontend(cpped::ipc::endpoint& endpoint, const boost::program_options::variables_map& options)
{
	OPEN_LOG_FILE("cpped_front.log");
	LOG("frontend process started");

	cpped::project project(endpoint);
	boost::optional<std::string> file_to_open;

	if (options.count("cmake"))
	{
		project.open_cmake_project(options["cmake"].as<std::string>());
	}
	else
	{
		auto files = options["file"].as<std::vector<std::string>>();
		file_to_open = files[0];
	}


	::setlocale(LC_ALL, "en_EN.utf-8");
	nct::ncurses_env env;

	cpped::utils::event_loop event_loop;
	nct::window_manager window_manager;
	cpped::style_manager styles;
	cpped::clipboard clipboard;
	cpped::main_window main_window(project, window_manager, styles);
	cpped::editor_window& editor = main_window.get_current_editor();

	if (file_to_open)
	{
		editor.open_file(*file_to_open);
	}

	main_window.set_fullscreen(true);
	main_window.set_active(); // so it recevies input
	main_window.show();

	window_manager.render_windows();
	cpped::utils::file_monitor observed_stdin(STDIN_FILENO, [&]() { window_manager.stdin_readable(); window_manager.render_windows(); });
	cpped::utils::file_monitor observed_pipe(endpoint.get_fd(), [&]() { endpoint.receive_message(); window_manager.render_windows(); });

	event_loop.run();
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

	cpped::ipc::subprocess backend_process = cpped::ipc::subprocess::fork(cpped::backend::run_backend_process);
	run_frontend(backend_process.get_endpoint(), vm);
}
