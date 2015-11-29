#include "document.hh"

#include <fstream>

namespace cpped { namespace  document {

void document::load_from_raw_data(std::vector<char> data)
{
	raw_data = std::move(data);
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

}}
