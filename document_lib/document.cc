#include "document.hh"

#include <iostream>

namespace cpped { namespace  document {

namespace fs = boost::filesystem;

document::document()
{
	// init empty document data
	data_.emplace_back();
	data_.back().data.init_empty();
	data_.back().version = 0;
	current_data_ = data_.begin();
}

document::~document()
{
}

void document::load_from_raw_data(const std::string& data, const fs::path& path)
{
	file_name_ = fs::absolute(path);

	data_.clear();
	data_.emplace_back();
	data_.back().data.load_from_raw_data(data);
	data_.back().version = 0;
	current_data_ = data_.begin();
	last_version_ = 0;
}

void document::load_from_raw_data(const std::string& data, const boost::filesystem::path& path, const token_data& tokens)
{
	load_from_raw_data(data, path);
	current_data_->data.set_tokens(tokens);
}

void document::load_from_file(const boost::filesystem::path& path)
{
	file_name_ = boost::filesystem::absolute(path);

	data_.clear();
	data_.emplace_back();
	data_.back().data.load_from_file(file_name_.string());
	data_.back().version = 0;
	current_data_ = data_.begin();
	last_version_ = 0;
}

document_position document::insert(document_position pos, const std::string& text)
{
	erase_redo();

	// add new at the end
	data_.emplace_back();
	data_.back().version = ++last_version_;
	auto final_pos = data_.back().data.copy_inserting(current_data_->data, pos, text);
	current_data_++;

	crop_history();

	_has_unsaved_changes = true;

	document_changed_signal();

	return final_pos;
}

void document::remove(document_range r)
{
	erase_redo();

	data_.emplace_back();
	data_.back().version = ++last_version_;
	data_.back().data.copy_removing(current_data_->data, r);
	current_data_++;

	_has_unsaved_changes = true;

	crop_history();

	document_changed_signal();
}

document_position document::remove_before(document_position pos, unsigned count)
{
	document_position begin = current_data_->data.shift_back(pos, count);
	remove(document_range{begin, pos});
	return begin;
}

document_position document::remove_after(document_position pos, unsigned count)
{
	document_position end = current_data_->data.shift_forward(pos, count);
	remove(document_range{pos, end});
	return end;
}

void document::parse_language()
{
	// TODO
}

std::string document::to_string() const
{
	return current_data_->data.to_string();
}

void document::set_tokens(std::uint64_t version, const token_data& tokens)
{
	if (current_data_->version == version)
	{
		current_data_->data.set_tokens(tokens);
		tokens_updated_signal();
	}
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
