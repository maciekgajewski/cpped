#include <iostream>
#include <stdexcept>

#include "document_lib/clang.hh"
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

	tu.parse(idx, filename, doc.get_raw_data().data(), doc.get_raw_data().size());

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
	unsigned complete_column = last_line.length();


	std::cout << "file end: " << complete_line << ":" << complete_column << std::endl;

	clang::code_completion_results completion_results = tu.code_complete_at(filename, end_of_file_location.line, end_of_file_location.column, doc.get_raw_data().data(), doc.get_raw_data().size());

	for(const clang::code_completion_result& result : completion_results)
	{
		std::cout << "* result, cursor kind=" << result.get_cursor_kind_as_string() << std::endl;

		clang::completion_string cs = result.get_completion_string();
		for(unsigned i = 0; i < cs.get_num_chunks(); i++)
		{
			std::cout << "  - " << cs.get_chunk_text(i) << std::endl;
		}
	}

	std::cout << "=== END ==" << std::endl;
}
