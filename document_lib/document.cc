#include "document.hh"

#include <boost/tokenizer.hpp>

#include <fstream>

namespace cpped { namespace  document {

void document::load_from_raw_data(const std::string& data)
{
	raw_data.assign(data.begin(), data.end());
	parse_raw_buffer();
}

void document::load_from_file(const std::string& path)
{
	std::fstream f(path, std::ios_base::in);
	if (f.fail())
	{
		throw std::runtime_error("Error opening file");
	}

	raw_data.clear();

	const unsigned bufsize = 4096;
	char buf[bufsize];
	while(!f.eof())
	{
		auto c = f.readsome(buf, bufsize);
		raw_data.insert(raw_data.end(), buf, buf+c);
	}

	parse_raw_buffer();
}

struct range
{
	std::vector<char>::const_iterator begin;
	std::vector<char>::const_iterator end;

	void assign(std::vector<char>::const_iterator b, std::vector<char>::const_iterator e)
	{
		begin = b;
		end = e;
	}
};

void document::parse_raw_buffer()
{
	lines.clear();

	// tokenize by lines
	boost::char_separator<char> endline_sep("\n\r");
	boost::tokenizer<decltype(endline_sep), std::vector<char>::const_iterator, range> tokens(raw_data, endline_sep);

	for(const range& token : tokens)
	{
	}
}


}}
