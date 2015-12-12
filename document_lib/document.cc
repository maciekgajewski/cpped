#include "document.hh"
#include "clang.hh"

#include <iostream>

namespace cpped { namespace  document {

document::document()
{
	// init empty document data
	data_.emplace_back();
	data_.back().init_empty();
	current_data_ = data_.begin();
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
	erase_redo();

	// add new at the end
	data_.emplace_back();
	auto final_pos = data_.back().copy_inserting(*current_data_, pos, text);
	current_data_++;

	crop_history();

	_has_unsaved_changes = true;

	return final_pos;
}

void document::remove(range r)
{
	erase_redo();

	data_.emplace_back();
	data_.back().copy_removing(*current_data_, r);
	current_data_++;

	_has_unsaved_changes = true;

	crop_history();
}

position document::remove_before(position pos, unsigned count)
{
	position begin = current_data_->shift_back(pos, count);
	remove(range{begin, pos});
	return begin;
}

position document::remove_after(position pos, unsigned count)
{
	position end = current_data_->shift_forward(pos, count);
	remove(range{pos, end});
	return end;
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

void document::erase_redo()
{
	auto next = current_data_;
	next++;
	data_.erase(next, data_.end());

}

void document::crop_history()
{
	static const unsigned MAX_HISTORY = 32;
	if (data_.size() > MAX_HISTORY)
		data_.pop_front();

}




}}
