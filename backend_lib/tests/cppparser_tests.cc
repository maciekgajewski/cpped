#include "backend_lib/cpp_tokens.hh"

#include "document_lib/document_data.hh"

#include "clang_lib/clang.hh"

#include <boost/test/unit_test.hpp>

#include <iostream>

namespace cpped { namespace backend { namespace test {

document::token_data parse_document_data(const document::document_data& d)
{
	clang::index idx(0, 0);
	clang::translation_unit tu;
	tu.parse(idx, "code.cc", d.get_raw_data().data(), d.get_raw_data().size(), {}, clang::translation_unit::full_parsing_options());

	document::token_data dt;
	dt.tokens = get_cpp_tokens(tu, "code.cc", d.get_raw_data());

	return dt;
}

BOOST_AUTO_TEST_SUITE(cpp_parser_tests)

BOOST_AUTO_TEST_CASE(simple_test)
{
	std::string code ="int x=7;";

	document::document_data d;
	d.load_from_raw_data(code);
	auto token_list = parse_document_data(d);
	d.set_tokens(token_list);

	int lines = 0;
	int tokens = 0;
	document::token_type expected_tokens[] = {
		document::token_type::keyword,	// int
		document::token_type::none,		// (space)
		document::token_type::none,		// x
		document::token_type::none,		// =
		document::token_type::literal,	// 7
		document::token_type::none,		// ;
		};

	d.for_lines(0, 1, [&](const document::line_data& line)
	{
		lines++;
		line.for_each_token([&](const document::line_token& token)
		{
			BOOST_REQUIRE_LT(tokens, sizeof(expected_tokens)/sizeof(document::token_type));
			BOOST_CHECK_EQUAL(expected_tokens[tokens], token.type);
			tokens++;
		});
	});
	BOOST_CHECK_EQUAL(lines, 1);
}

BOOST_AUTO_TEST_CASE(multiline_comment)
{
	std::string code =
R"(/*345
123456789
*/)";

	document::document_data d;
	d.load_from_raw_data(code);
	auto token_list = parse_document_data(d);
	d.set_tokens(token_list);

	BOOST_REQUIRE_EQUAL(d.get_line_count(), 3);

	auto& l0_tokens = d.get_line(0).get_tokens();
	BOOST_REQUIRE_EQUAL(l0_tokens.size() , 1);
	BOOST_CHECK_EQUAL(l0_tokens[0] , (document::line_token{0, 5, document::token_type::comment}));

	auto& l1_tokens = d.get_line(1).get_tokens();
	BOOST_REQUIRE_EQUAL(l1_tokens.size() , 1);
	BOOST_CHECK_EQUAL(l1_tokens[0] , (document::line_token{0, 9, document::token_type::comment}));

	auto& l2_tokens = d.get_line(2).get_tokens();
	BOOST_REQUIRE_EQUAL(l2_tokens.size() , 1);
	BOOST_CHECK_EQUAL(l2_tokens[0] , (document::line_token{0, 2, document::token_type::comment}));
}


/*
BOOST_AUTO_TEST_CASE(feral_literal_test)
{
	std::string code ="struct X { const char* a=\"alamakota\"; };";

	document d;
	d.load_from_raw_data(code, "code.cc");

	clang::index index(0, 0);
	clang::translation_unit tu(index, "code.cc", d.get_raw_data().data(), d.get_raw_data().size());

	clang::source_file file = tu.get_file("code.cc");

	// full file tokenizer. doing it line-by-line is not a good approach
	clang::source_location file_begin = tu.get_location_for_offset(file, 0);
	clang::source_location file_end = tu.get_location_for_offset(file, d.get_raw_data().size());
	clang::source_range range(file_begin, file_end);
	clang::token_list tokens = tu.tokenize(range);
	tokens.annotate_tokens();

	for(const clang::token& token : tokens)
	{
		if (token.get_kind() == CXToken_Literal)
		{
			clang::cursor cursor = token.get_associated_cursor();

			std::cout << "cursor kind for 'literal' token: " << cursor.get_kind_as_string() << std::endl;
			cursor.visit_children([&](const clang::cursor& p, const clang::cursor& c)
			{
				std::cout << "Child cursor kind: " << c.get_kind_as_string() << std::endl;
				return CXChildVisit_Recurse;
			});

			auto lp1 = cursor.get_lexical_parent();
			std::cout << "lexical parent1=" << lp1.get_kind_as_string() << std::endl;

			auto lp2 = lp1.get_lexical_parent();
			std::cout << "lexical parent2=" << lp2.get_kind_as_string() << std::endl;


			lp2.visit_children([&](const clang::cursor& c, const clang::cursor& p)
			{
				std::cout << "lexical parent's child kind: " << c.get_kind_as_string() << std::endl;
				return CXChildVisit_Recurse;
			});
		}
	}
}
*/

BOOST_AUTO_TEST_CASE(completion_test)
{
	std::string code =
R"(#include <string>
void fun() {
	std::string s;
	auto text="aaa bb
)";

	document::document_data d;
	d.load_from_raw_data(code);

	clang::index idx(0, 0);
	clang::translation_unit tu;
	tu.parse(idx, "code.cc", d.get_raw_data().data(), d.get_raw_data().size(), {}, clang::translation_unit::full_parsing_options());

	document::token_data dt;
	dt.tokens =  get_cpp_tokens(tu, "code.cc", d.get_raw_data());
	d.set_tokens(dt);
}

BOOST_AUTO_TEST_SUITE_END()

}}}
