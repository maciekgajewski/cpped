#define BOOST_TEST_MODULE document_tests

#include "document_lib/document.hh"

#include <boost/test/included/unit_test.hpp>

namespace cpped { namespace document { namespace test {

BOOST_AUTO_TEST_SUITE( document_tests )

BOOST_AUTO_TEST_CASE(document_lines)
{
	std::string code =
R"(1
22
333
4444
55555
666666)";


	document doc;
	doc.load_from_raw_data(code, "fake.cc");

	BOOST_CHECK_EQUAL(6, doc.get_line_count());
	for(unsigned i = 0; i < doc.get_line_count(); ++i)
	{
		BOOST_CHECK_EQUAL(i+1, doc.get_line(i).get_length());
	}
}

BOOST_AUTO_TEST_CASE(cpp_parsing)
{
	std::string code =
R"(#include <string>

// hello boss
const char* s = "a\
lamakaota";

struct X {
  int i = 666;
  std::string s = "string literal";
};)";


	document doc;
	doc.load_from_raw_data(code, "fake.cc");

	doc.parse_language(); // this test onlt has value if there is any debug printing. At least we'll see if it doesn't crash...
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(line_tests)
BOOST_AUTO_TEST_CASE(no_tokens)
{
	std::string text = "abcdefg gijklm";
	document_line line(const_cast<char*>(text.data()), text.length());

	int token_count = 0;
	line.for_each_token([&](const line_token& t)
	{
		token_count++;
		BOOST_CHECK_EQUAL(t.type, token_type::none);
		BOOST_CHECK_EQUAL(t.begin, 0u);
		BOOST_CHECK_EQUAL(t.end, text.length());
	});
	BOOST_CHECK_EQUAL(token_count, 1);
}

BOOST_AUTO_TEST_CASE(back_to_back_tokens)
{
	std::string text = "11222333";
	document_line line(const_cast<char*>(text.data()), text.length());

	token_type expected_types[] = {token_type::keyword, token_type::comment, token_type::literal};

	line.push_back_token(line_token{0, 2, expected_types[0]});
	line.push_back_token(line_token{2, 5, expected_types[1]});
	line.push_back_token(line_token{5, 8, expected_types[2]});

	int token_count = 0;
	unsigned current = 0;
	line.for_each_token([&](const line_token& t)
	{
		BOOST_CHECK_EQUAL(t.type, expected_types[token_count]);
		BOOST_CHECK_EQUAL(t.begin, current);
		current = t.end;
		token_count++;
	});
	BOOST_CHECK_EQUAL(token_count, 3);
}

BOOST_AUTO_TEST_CASE(token_gap)
{
	std::string text = "11222333";
	document_line line(const_cast<char*>(text.data()), text.length());

	token_type expected_types[] = {token_type::keyword, token_type::none, token_type::literal};

	line.push_back_token(line_token{0, 2, expected_types[0]});
	//line.push_back_token(line_token{2, 5, expected_types[1]});
	line.push_back_token(line_token{5, 8, expected_types[2]});

	int token_count = 0;
	unsigned current = 0;
	line.for_each_token([&](const line_token& t)
	{
		BOOST_CHECK_EQUAL(t.type, expected_types[token_count]);
		BOOST_CHECK_EQUAL(t.begin, current);
		current = t.end;
		token_count++;
	});
	BOOST_CHECK_EQUAL(token_count, 3);
}

BOOST_AUTO_TEST_CASE(token_in_the_middle)
{
	std::string text = "11222333";
	document_line line(const_cast<char*>(text.data()), text.length());

	token_type expected_types[] = {token_type::none, token_type::literal, token_type::none};

	//line.push_back_token(line_token{0, 2, expected_types[0]});
	line.push_back_token(line_token{2, 5, expected_types[1]});
	//line.push_back_token(line_token{5, 8, expected_types[2]});

	int token_count = 0;
	unsigned current = 0;
	line.for_each_token([&](const line_token& t)
	{
		BOOST_CHECK_EQUAL(t.type, expected_types[token_count]);
		BOOST_CHECK_EQUAL(t.begin, current);
		current = t.end;
		token_count++;
	});
	BOOST_CHECK_EQUAL(token_count, 3);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(cpp_parser_tests)

BOOST_AUTO_TEST_CASE(simple_test)
{
	std::string code ="int x=7;";

	document d;
	d.load_from_raw_data(code, "code.cc");
	d.parse_language();

	int lines = 0;
	int tokens = 0;
	token_type expected_tokens[] = {
		token_type::keyword,	// int
		token_type::none,		// (space)
		token_type::none,		// x
		token_type::none,		// =
		token_type::literal,	// 7
		token_type::none,		// ;
		};

	d.for_lines(0, 1, [&](const document_line& line)
	{
		lines++;
		line.for_each_token([&](const line_token& token)
		{
			BOOST_REQUIRE_LT(tokens, sizeof(expected_tokens)/sizeof(token_type));
			BOOST_CHECK_EQUAL(expected_tokens[tokens], token.type);
			tokens++;
		});
	});
	BOOST_CHECK_EQUAL(lines, 1);
}

BOOST_AUTO_TEST_SUITE_END()

}}}
