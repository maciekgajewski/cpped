#include "completion.hh"

namespace cpped { namespace backend {

std::vector<messages::completion_record> process_completion_results(const clang::code_completion_results& results)
{
	// intermediate
	std::vector<std::pair<unsigned, messages::completion_record>> converted;

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

		// check if text was already seen, to avoid duplicates
		auto it = std::find_if(converted.begin(), converted.end(),
			[&](const auto& pair)
			{
				return pair.second.text == text;
			});
		if (it == converted.end())
		{
			converted.push_back({cs.get_priority(), {text, hint}});
		}
	}

	std::sort(converted.begin(), converted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

	std::vector<messages::completion_record> output;
	output.reserve(converted.size());

	for(auto& pair : converted)
	{
		output.push_back(std::move(pair.second));
	}

	return output;
}

}}
