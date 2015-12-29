#pragma once

#include "messages.hh"

#include "clang_lib/clang.hh"

#include <vector>

namespace cpped { namespace backend {

std::vector<messages::completion_record> process_completion_results(
	const clang::code_completion_results& results);

}}
