#include "open_file.hh"

#include "compilation_unit.hh"

#include "utils_lib/log.hh"

#include <fstream>

namespace cpped { namespace backend {

namespace  fs = boost::filesystem;

open_file::open_file(const fs::path& path)
	: path_(path)
{
	if (fs::exists(path))
	{
		load_from_disk();
		was_new_ = false;
	}
	else
	{
		was_new_ = true;
	}
}

void open_file::save()
{
	// create temporary file
	fs::path temp_dir = fs::temp_directory_path();
	fs::path temp_file = fs::unique_path(temp_dir/"cpped-%%%%-%%%%-%%%%-%%%%.tmp");

	std::ofstream file(temp_file.string());
	file.write(data_.data(), data_.size());
	if (!file.good())
	{
		fs::remove(temp_file);
		throw std::runtime_error("Error writing to temp file: " + temp_file.string());
	}
	file.close();

	fs::rename(temp_file, path_);
}

document::token_data open_file::parse(const std::vector<CXUnsavedFile>& unsaved_data)
{
	document::token_data tokens;

	LOG("Reparsing file " << path_);

	if (unit_)
	{
		LOG("File has compilation unit...");
		if (unit_->needs_parsing())
		{
			LOG("... that needs parsing");
			unit_->parse(unsaved_data, parse_mode::full);
		}
		else
		{
			LOG("... that may need reparsing");
			unit_->reparse(unsaved_data);
		}
		tokens = unit_->get_tokens_with_diagnostics(path_, data_);
	}

	return tokens;
}

std::vector<messages::completion_record> open_file::complete_at(
	const std::vector<CXUnsavedFile>& unsaved_data,
	const document::document_position& pos)
{
	std::vector<messages::completion_record> result;

	if (unit_)
	{
		result = unit_->complete_at(unsaved_data, path_, pos);
	}

	return result;
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
