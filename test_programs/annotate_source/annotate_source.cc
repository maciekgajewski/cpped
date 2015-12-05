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
	doc.load_from_file(argv[1]);

	clang::index idx(0, 0);

	clang::translation_unit tu;

	tu.parse(idx, argv[1], doc.get_raw_data().data(), doc.get_raw_data().size());

	clang::source_file file = tu.get_file(argv[1]);
	clang::source_location file_begin = tu.get_location_for_offset(file, 0);
	clang::source_location file_end = tu.get_location_for_offset(file, doc.get_raw_data().size());

	clang::source_range range(file_begin, file_end);
	clang::token_list tokens = tu.tokenize(range);
	tokens.annotate_tokens();

	unsigned line_no = 0;
	for(const clang::token& token : tokens)
	{
		clang::source_range extent = tokens.get_token_extent(token);
		clang::source_location::info token_begin = extent.get_start().get_location_info();
		clang::source_location::info token_end = extent.get_end().get_location_info();

		// print source lines if unprinted
		for (;token_end.line > line_no; line_no++)
		{
			std::cout << doc.get_line(line_no).to_string() << std::endl;
		}

		// print token
		std::cout << "(" << token_begin.line << ":" << token_begin.column << " - " << token_end.line << ":" << token_end.column << ") ";
		std::cout << token.get_kind_name() << " " << token.get_associated_cursor().get_kind_as_string() << " ";
	}
	std::cout << std::endl;
}
