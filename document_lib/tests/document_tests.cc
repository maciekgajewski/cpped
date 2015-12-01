#include "document_lib/document.hh"

#include <boost/test/unit_test.hpp>

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
	document doc;
	doc.load_from_raw_data(text, "");
	document_line& line = doc.get_line(0);

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
	document doc;
	doc.load_from_raw_data(text, "");
	document_line& line = doc.get_line(0);

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
	document doc;
	doc.load_from_raw_data(text, "");
	document_line& line = doc.get_line(0);

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
	document doc;
	doc.load_from_raw_data(text, "");
	document_line& line = doc.get_line(0);

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

BOOST_AUTO_TEST_CASE(insert_character_no_token)
{
	std::string text = "111\n222\n333";
	document doc;
	doc.load_from_raw_data(text, "filename");

	BOOST_REQUIRE_EQUAL(3, doc.get_line_count());

	std::string l0 = doc.get_line(0).to_string();
	std::string l1 = doc.get_line(1).to_string();
	std::string l2 = doc.get_line(2).to_string();

	auto& line1 = doc.get_line(1);
	line1.insert(1, 'x');

	// no changes to previous and subsequent line
	BOOST_CHECK_EQUAL(doc.get_line(0).to_string(), l0);
	BOOST_CHECK_EQUAL(doc.get_line(2).to_string(), l2);

	// line is 1 changed
	BOOST_CHECK_EQUAL(doc.get_line(1).to_string(), "2x22");
}

BOOST_AUTO_TEST_CASE(insert_character_into_token)
{
	std::string text = "111\n22xx\n333";
	document doc;
	doc.load_from_raw_data(text, "filename");

	BOOST_REQUIRE_EQUAL(3, doc.get_line_count());

	auto& line1 = doc.get_line(1);

	line1.push_back_token({0, 2, token_type::comment});
	line1.push_back_token({2, 4, token_type::literal});

	line1.insert(1, 'x');

	const std::vector<line_token>& tokens = line1.get_tokens();

	BOOST_REQUIRE_EQUAL(2, tokens.size());

	// expect token 0 to be stretched, token 1 to be shifted
	BOOST_CHECK_EQUAL(tokens[0].begin, 0);
	BOOST_CHECK_EQUAL(tokens[0].end, 3);

	BOOST_CHECK_EQUAL(tokens[1].begin, 3);
	BOOST_CHECK_EQUAL(tokens[1].end, 5);
}

BOOST_AUTO_TEST_CASE(insert_newline_no_tokens)
{
	std::string text = "111\n22xx\n333";
	document doc;
	doc.load_from_raw_data(text, "");

	BOOST_REQUIRE_EQUAL(3, doc.get_line_count());

	auto& line1 = doc.get_line(1);
	BOOST_CHECK_EQUAL(line1.get_length(), 4);

	line1.insert(2, '\n');

	BOOST_REQUIRE_EQUAL(4, doc.get_line_count());
	BOOST_CHECK_EQUAL(doc.get_line(0).get_length(), 3);
	BOOST_CHECK_EQUAL(doc.get_line(1).get_length(), 2);
	BOOST_CHECK_EQUAL(doc.get_line(2).get_length(), 2);
	BOOST_CHECK_EQUAL(doc.get_line(3).get_length(), 3);

	BOOST_CHECK_EQUAL(doc.get_line(0).to_string(), "111");
	BOOST_CHECK_EQUAL(doc.get_line(1).to_string(), "22");
	BOOST_CHECK_EQUAL(doc.get_line(2).to_string(), "xx");
	BOOST_CHECK_EQUAL(doc.get_line(3).to_string(), "333");
}

BOOST_AUTO_TEST_CASE(insert_newline_between_tokens)
{
	std::string text = "1122\n33xx44\n5566";
	document doc;
	doc.load_from_raw_data(text, "");

	BOOST_REQUIRE_EQUAL(3, doc.get_line_count());

	doc.get_line(0).push_back_token(line_token{0, 2, token_type::keyword}); // 11
	doc.get_line(0).push_back_token(line_token{2, 4, token_type::literal}); // 22

	doc.get_line(1).push_back_token(line_token{0, 2, token_type::preprocessor}); // 33
	doc.get_line(1).push_back_token(line_token{4, 6, token_type::type}); // 44

	doc.get_line(2).push_back_token(line_token{0, 2, token_type::keyword}); // 55
	doc.get_line(2).push_back_token(line_token{2, 4, token_type::literal}); // 66

	auto& line1 = doc.get_line(1);
	BOOST_CHECK_EQUAL(line1.get_length(), 6);

	line1.insert(3, '\n');

	BOOST_REQUIRE_EQUAL(4, doc.get_line_count());

	BOOST_CHECK_EQUAL(doc.get_line(0).get_length(), 4);
	BOOST_CHECK_EQUAL(doc.get_line(1).get_length(), 3);
	BOOST_CHECK_EQUAL(doc.get_line(2).get_length(), 3);
	BOOST_CHECK_EQUAL(doc.get_line(3).get_length(), 4);

	auto& l0_tokens= doc.get_line(0).get_tokens();
	BOOST_REQUIRE_EQUAL(l0_tokens.size(), 2);
	BOOST_CHECK_EQUAL(l0_tokens[0], (line_token{0, 2, token_type::keyword}));
	BOOST_CHECK_EQUAL(l0_tokens[1], (line_token{2, 4, token_type::literal}));

	auto& l1_tokens = doc.get_line(1).get_tokens();
	BOOST_REQUIRE_EQUAL(l1_tokens.size(), 1);
	BOOST_CHECK_EQUAL(l1_tokens[0], (line_token{0, 2, token_type::preprocessor})); // 33

	auto& l2_tokens = doc.get_line(2).get_tokens();
	BOOST_REQUIRE_EQUAL(l2_tokens.size(), 1);
	BOOST_CHECK_EQUAL(l2_tokens[0], (line_token{1, 3, token_type::type})); // 44

	auto& l3_tokens = doc.get_line(3).get_tokens();
	BOOST_REQUIRE_EQUAL(l3_tokens.size(), 2);
	BOOST_CHECK_EQUAL(l3_tokens[0], (line_token{0, 2, token_type::keyword}));
	BOOST_CHECK_EQUAL(l3_tokens[1], (line_token{2, 4, token_type::literal}));
}

BOOST_AUTO_TEST_CASE(insert_newline_inside_token)
{
	std::string text = "1122\n333\n5566";
	document doc;
	doc.load_from_raw_data(text, "");

	BOOST_REQUIRE_EQUAL(3, doc.get_line_count());

	doc.get_line(0).push_back_token(line_token{0, 2, token_type::keyword}); // 11
	doc.get_line(0).push_back_token(line_token{2, 4, token_type::literal}); // 22

	doc.get_line(1).push_back_token(line_token{0, 3, token_type::preprocessor}); // 333

	doc.get_line(2).push_back_token(line_token{0, 2, token_type::keyword}); // 55
	doc.get_line(2).push_back_token(line_token{2, 4, token_type::literal}); // 66

	auto& line1 = doc.get_line(1);
	BOOST_CHECK_EQUAL(line1.get_length(), 3);

	line1.insert(2, '\n');

	BOOST_REQUIRE_EQUAL(4, doc.get_line_count());

	BOOST_CHECK_EQUAL(doc.get_line(0).get_length(), 4);
	BOOST_CHECK_EQUAL(doc.get_line(1).get_length(), 2);
	BOOST_CHECK_EQUAL(doc.get_line(2).get_length(), 1);
	BOOST_CHECK_EQUAL(doc.get_line(3).get_length(), 4);

	auto& l0_tokens= doc.get_line(0).get_tokens();
	BOOST_REQUIRE_EQUAL(l0_tokens.size(), 2);
	BOOST_CHECK_EQUAL(l0_tokens[0], (line_token{0, 2, token_type::keyword}));
	BOOST_CHECK_EQUAL(l0_tokens[1], (line_token{2, 4, token_type::literal}));

	auto& l1_tokens = doc.get_line(1).get_tokens();
	BOOST_REQUIRE_EQUAL(l1_tokens.size(), 1);
	BOOST_CHECK_EQUAL(l1_tokens[0], (line_token{0, 2, token_type::preprocessor})); // 33

	auto& l2_tokens = doc.get_line(2).get_tokens();
	BOOST_REQUIRE_EQUAL(l2_tokens.size(), 1);
	BOOST_CHECK_EQUAL(l2_tokens[0], (line_token{0, 1, token_type::preprocessor})); // 3

	auto& l3_tokens = doc.get_line(3).get_tokens();
	BOOST_REQUIRE_EQUAL(l3_tokens.size(), 2);
	BOOST_CHECK_EQUAL(l3_tokens[0], (line_token{0, 2, token_type::keyword}));
	BOOST_CHECK_EQUAL(l3_tokens[1], (line_token{2, 4, token_type::literal}));
}

BOOST_AUTO_TEST_SUITE_END()

}}}
