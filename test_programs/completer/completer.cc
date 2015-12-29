#include <iostream>
#include <stdexcept>

#include "clang_lib/clang.hh"
#include "clang_lib/flags.hh"

#include "document_lib/document.hh"

using namespace cpped;

int main(int argc, char** argv)
{
	if(argc < 2)
		throw std::runtime_error("param required");

	document::document doc;
	const char* filename = argv[1];
	doc.load_from_file(filename);

	clang::index idx(0, 0);

	clang::translation_unit tu;

	std::vector<std::string> args;
	clang::get_common_flags(args);

	std::vector<const char*> cmdline;
	cmdline.reserve(argc-2 + args.size());
	for(int i = 2; i < argc; i++)
		cmdline.push_back(argv[i]);
	for(const std::string& a : args)
		cmdline.push_back(a.c_str());

	tu.parse(idx, filename, doc.get_raw_data().data(), doc.get_raw_data().size(), cmdline);

	clang::source_file file = tu.get_file(filename);
	clang::source_location file_end = tu.get_location_for_offset(file, doc.get_raw_data().size());

	clang::source_location::info end_of_file_location = file_end.get_location_info();

	// print the file
	doc.for_lines(0, doc.get_line_count()-1, [&](const document::document_line& line)
	{
		std::cout << line.to_string() << std::endl;
	});

	std::string last_line = doc.get_line(doc.get_line_count()-2).to_string();
	for(char c : last_line)
	{
		if (c == '\t')
			std::cout << '\t';
		else
			std::cout << ' ';
	}
	std::cout << '^' << std::endl;

	std::cout << "===" << std::endl;
	unsigned complete_line = doc.get_line_count()-1;
	unsigned complete_column = last_line.length()+1;


	std::cout << "file end: " << complete_line << ":" << complete_column << std::endl;
	std::cout << "eof location: " << end_of_file_location.line << ":" << end_of_file_location.column << std::endl;

	clang::code_completion_results completion_results = tu.code_complete_at(filename, complete_line, complete_column, doc.get_raw_data().data(), doc.get_raw_data().size());

	struct result_record
	{
		std::string text;
		int prio;
	};
	std::vector<result_record> results;
	results.reserve(completion_results.size());

	for(const clang::code_completion_result& result : completion_results)
	{
		result_record r;
		clang::completion_string cs = result.get_completion_string();
		r.prio = cs.get_priority();
		for(unsigned i = 0; i < cs.get_num_chunks(); i++)
		{
			if (cs.get_chunk_kind(i) == CXCompletionChunk_TypedText)
			{
				r.text = cs.get_chunk_text(i).c_str();
			}
		}

		results.push_back(r);

//		clang::completion_string cs = result.get_completion_string();
//		std::cout << "* result, cursor kind=" << result.get_cursor_kind_as_string()
//			<< ", priority=" << cs.get_priority()
//			<< ", brief comment=" << cs.get_brief_comment()
//			<< std::endl;
//		for(unsigned i = 0; i < cs.get_num_chunks(); i++)
//		{
//			std::cout << "  - text='" << cs.get_chunk_text(i) << "', kind=" << clang::completion_chunk_kind_to_str(cs.get_chunk_kind(i)) << std::endl;
//		}
	}

	//std::sort(results.begin(), results.end(), [](const result_record& a, const result_record& b) { return a.prio < b.prio; });

///	unsigned n = std::min<unsigned>(30, results.size());
	for(unsigned i = 0; i < results.size(); i++)
	{
		std::cout << "* " << results[i].text << std::endl;
	}


	std::cout << "=== END ==" << std::endl;
}
