#include "open_file.hh"

#include <fstream>

namespace cpped { namespace backend {

namespace  fs = boost::filesystem;

open_file::open_file(const fs::path& path)
	: path_(path)
{
	load_from_disk();
}

void open_file::load_from_disk()
{
	assert(data_.empty());
	static const constexpr unsigned BUFSIZE = 8*1024;

	std::ifstream stream(path_.string());
	data_.reserve(BUFSIZE);

	if (stream.fail())
	{
		throw std::runtime_error("Failed to open file");
	}

	char buf[BUFSIZE];
	while(true)
	{
		auto sz = stream.readsome(buf, BUFSIZE);
		if (!stream.good())
		{
			throw std::runtime_error("Error reading file");
		}
		if (sz == 0)
			break;
		data_.insert(data_.end(), buf, buf + sz);
	}
}

}}
