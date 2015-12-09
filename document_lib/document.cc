#include "document.hh"
#include "clang.hh"

#include <iostream>

namespace cpped { namespace  document {

document::document()
{
	// init empty document data
	data_.emplace_back();
	data_.back().init_empty();
}

document::~document()
{
}

void document::load_from_raw_data(const std::string& data, const std::string& fake_path, std::unique_ptr<iparser>&& p)
{
	file_name_ = fake_path;

	data_.clear();
	data_.emplace_back();
	data_.back().load_from_raw_data(data);
	current_data_ = data_.begin();

	parser_ = std::move(p);
}

void document::load_from_file(const std::string& path, std::unique_ptr<iparser>&& p)
{
	file_name_ = path;

	data_.clear();
	data_.emplace_back();
	data_.back().load_from_file(path);
	current_data_ = data_.begin();

	parser_ = std::move(p);
}

position document::insert(position pos, const std::string& text)
{
	// erase any data ahead of current iterator
	auto next = current_data_;
	next++;
	data_.erase(next, data_.end());

	// add new at the end
	data_.emplace_back();
	auto final_pos = data_.back().copy_inserting(*current_data_, pos, text);
	current_data_ = next;

	// crop history
	static const unsigned MAX_HISTORY = 32;
	if (data_.size() > MAX_HISTORY)
		data_.pop_front();

	return final_pos;
}

void document::parse_language()
{
	if (parser_)
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		parser_->parse(*current_data_, file_name_);
		auto end_time = std::chrono::high_resolution_clock::now();

		last_parse_time_ = end_time - start_time;
	}

}

std::string document::to_string() const
{
	return current_data_->to_string();
}




}}
