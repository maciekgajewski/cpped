#include "document.hh"
#include "clang.hh"

#include <iostream>

namespace cpped { namespace  document {

document::document()
{
	// add one empty line
	lines_.emplace_back(*this, nullptr, 0);
}

document::~document()
{
}

void document::load_from_raw_data(const std::string& data, const std::string& fake_path, std::unique_ptr<iparser>&& p)
{
	file_name = fake_path;

	data_.clear();
	data_.emplace_back();
	data_.back().load_from_raw_data(data);
	current_data_ = data_.begin();

	parser = std::move(p);
}

void document::load_from_file(const std::string& path, std::unique_ptr<iparser>&& p)
{
	file_name = path;

	data_.clear();
	data_.emplace_back();
	data_.back().load_from_file(path);
	current_data_ = data_.begin();

	parser = std::move(p);
}

//std::vector<char>::const_iterator document::postion_to_offset(position pos) const
//{

//}

position document::insert(position pos, const std::string& text)
{
	// TODO rewrtie
//	for(char c : text)
//	{
//		auto& doc_line = get_line(pos.line);
//		doc_line.insert(pos.column, c);
//		if (c == '\n')
//		{
//			pos.column = 0;
//			pos.line++;
//		}
//		else
//		{
//			pos.column++;
//		}
//	}
//	return pos;

	// calculate number of lines of the inserted text
//	unsigned inserted_lines = std::count(text.begin(), text.end(), '\n');

//	// create new data buffers
//	std::vector<char> new_raw_data;
//	new_raw_data.reserve(raw_data_ + text.length());

//	std::vector<document_line> new_lines;
//	new_lines.reserve(lines_.size() + inserted_lines);

//	// copy data, inserting the new chunk
//	auto insert_offset = postion_to_offset(pos);

//	// copy all lines from [0, pos.line) without changes
//	auto end = lines_.begin() + pos.line;
//	for (auto it = lines_.begin(); it != end; ++it)
//	{
//		new_lines.emplace_back(
//	}
}

void document::parse_language()
{
	if (parser)
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		parser->parse(*this);
		auto end_time = std::chrono::high_resolution_clock::now();

		last_parse_time = end_time - start_time;
	}

}

std::string document::to_string() const
{
	return current_data_->to_string();
}




}}
