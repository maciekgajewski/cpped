#include "completion.hh"

namespace cpped { namespace backend {

std::vector<messages::completion_record> process_completion_results(const clang::code_completion_results& results)
{
	std::vector<messages::completion_record> output;

	std::string hint;
	std::string text;
	for(const clang::code_completion_result& result : results)
	{
		hint.clear();
		text.clear();
		clang::completion_string cs = result.get_completion_string();
		for(auto i = 0u; i < cs.get_num_chunks(); i++)
		{
			clang::string s = cs.get_chunk_text(i);
			if (cs.get_chunk_kind(i) == CXCompletionChunk_TypedText)
			{
				text = s.c_str();
			}
			hint += s.c_str();
		}
	}
}

}}
