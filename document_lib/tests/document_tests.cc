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
	doc.load_from_raw_data(code);

	BOOST_CHECK_EQUAL(6, doc.get_line_count());
	for(unsigned i = 0; i < doc.get_line_count(); ++i)
	{
		BOOST_CHECK_EQUAL(i+1, doc.get_line(i).get_length());
	}
}

BOOST_AUTO_TEST_SUITE_END()
}}}
