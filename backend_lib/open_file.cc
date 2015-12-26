#include "open_file.hh"

#include "log.hh"
#include "compilation_unit.hh"

#include <fstream>

namespace cpped { namespace backend {

namespace  fs = boost::filesystem;

open_file::open_file(const fs::path& path)
	: path_(path)
{
	load_from_disk();
}

std::vector<document::token> open_file::parse(const std::vector<CXUnsavedFile>& unsaved_data)
{
	std::vector<document::token> tokens;

	LOG("Reparsing file " << path_);

	if (unit_)
	{
		LOG("File has compilation unit...");
		if (unit_->needs_parsing())
		{
			LOG("... that needs parsing");
			unit_->parse(unsaved_data);
		}
		else
		{
			LOG("... that may need reparsing");
			unit_->reparse(unsaved_data);
		}
		tokens = unit_->get_tokens_for_file(path_, data_);
	}

	return tokens;
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
