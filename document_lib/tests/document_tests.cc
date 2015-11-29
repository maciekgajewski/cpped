#define BOOST_TEST_MODULE document_tests

#include "document_lib/document.hh"

#include <boost/test/included/unit_test.hpp>

namespace cpped { namespace document { namespace test {

BOOST_AUTO_TEST_SUITE( document_tests )

BOOST_AUTO_TEST_CASE(document_lines)
{
	std::string code =
R"(line 1
line 2
line 3
line 4
line 5
line 6)";


	document doc;
	doc.load_from_raw_data(code);

	BOOST_CHECK_EQUAL(6, doc.get_line_count());
}

BOOST_AUTO_TEST_SUITE_END()
}}}
