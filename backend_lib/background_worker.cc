#include "background_worker.hh"

#include "background_worker_messages.hh"

#include "utils_lib/log.hh"
#include "utils_lib/event_loop.hh"

#include "clang_lib/clang.hh"
#include "clang_lib/utils.hh"

#include <boost/range/algorithm.hpp>

#include <string>

namespace cpped { namespace backend {

static worker_messages::parse_file_result parse_file(const worker_messages::parse_file_request& req)
{
	std::vector<const char*> cmdline;
	cmdline.reserve(req.commanline_params.size());
	boost::transform(
		req.commanline_params,
		std::back_inserter(cmdline),
		[](const std::string& s) { return s.c_str(); });

	clang::translation_unit tu;
	clang::index idx(0, 0);
	unsigned opts = CXTranslationUnit_SkipFunctionBodies|CXTranslationUnit_PrecompiledPreamble|CXTranslationUnit_DetailedPreprocessingRecord;
	tu.parse(idx, req.file.c_str(), {}, cmdline, opts);

	auto includes = clang::get_includes(tu);

	worker_messages::parse_file_result result;
	result.file = req.file;
	result.includes.assign(includes.begin(), includes.end());

	return result;
}

int background_worker_entry(unsigned worker_number, ipc::endpoint& ep)
{
	OPEN_LOG_FILE("cpped_worker_" + std::to_string(worker_number) + ".log");

	ep.register_message_handler<worker_messages::parse_file_request>(
		[&](const worker_messages::parse_file_request& req)
		{
			try
			{
				LOG("Recevied parse file request for: " << req.file << "params: " << req.commanline_params.size());
				auto reply = parse_file(req);
				LOG("Sending parse file reply for: " << req.file << ", includes: " << reply.includes.size());
				ep.send_message(reply);
			}
			catch(const std::exception& e)
			{
				LOG("Error parsing file " << req.file << " : " << e.what());
				ep.send_message(worker_messages::parse_file_request{});
			}
		});

	utils::event_loop loop;
	utils::file_monitor endpoint_monitor(ep.get_fd(),
		[&]()
		{
			ep.receive_message();
		});

	try
	{
		loop.run();
	}
	catch(const std::exception& e)
	{
		LOG("Error in main loop: " << e.what());
		return 1; // do nothing, just die
	}

	return 0;
}

}}
