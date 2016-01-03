#include "document.hh"

#include <iostream>

namespace cpped { namespace  document {

namespace fs = boost::filesystem;

document::document()
{
	// init empty document data
	data_.emplace_back(versioned_data{std::make_unique<document_data>(), 0});
	data_.back().data->init_empty();
	current_data_ = data_.begin();
}

document::~document()
{
}

void document::load_from_raw_data(const std::string& data, const fs::path& path)
{
	file_name_ = fs::absolute(path);

	data_.clear();
	data_.emplace_back(versioned_data{std::make_unique<document_data>(), 0});
	data_.back().data->load_from_raw_data(data);
	current_data_ = data_.begin();
	last_version_ = 0;
}

void document::load_from_raw_data(const std::string& data, const boost::filesystem::path& path, const token_data& tokens)
{
	load_from_raw_data(data, path);
	current_data_->data->set_tokens(tokens);
}

void document::load_from_file(const boost::filesystem::path& path)
{
	file_name_ = boost::filesystem::absolute(path);

	data_.clear();
	data_.emplace_back(versioned_data{std::make_unique<document_data>(), 0});
	data_.back().data->load_from_file(file_name_.string());
	current_data_ = data_.begin();
	last_version_ = 0;
}

document_edit::document_edit(document& doc)
: document_(doc), current_data_(&doc.get_data())
{
}

document_position document_edit::insert(document_position pos, const std::string& text)
{
	auto changed = std::make_unique<document_data>();

	auto final_pos = changed->copy_inserting(*current_data_, pos, text);

	changed_data_ = std::move(changed);
	current_data_ = changed_data_.get();

	return final_pos;
}

void document_edit::remove(document_range r)
{
	auto changed = std::make_unique<document_data>();

	changed->copy_removing(*current_data_, r);

	changed_data_ = std::move(changed);
	current_data_ = changed_data_.get();
}

document_position document_edit::remove_before(document_position pos, unsigned count)
{
	document_position begin = current_data_->shift_back(pos, count);
	remove(document_range{begin, pos});
	return begin;
}

document_position document_edit::remove_after(document_position pos, unsigned count)
{
	document_position end = current_data_->shift_forward(pos, count);
	remove(document_range{pos, end});
	return end;
}

void document_edit::commit(const document_position& cursor_pos)
{
	if (changed_data_)
	{
		document_.commit_change(std::move(changed_data_), cursor_pos);
	}
}

void document::parse_language()
{
	// TODO
}

std::string document::to_string() const
{
	return current_data_->data->to_string();
}

void document::set_tokens(std::uint64_t version, const token_data& tokens)
{
	if (current_data_->version == version)
	{
		current_data_->data->set_tokens(tokens);
		tokens_updated_signal();
	}
}

void document::commit_change(std::unique_ptr<document_data>&& new_data, const document_position& cursor_pos)
{
	erase_redo();

	// add new at the end
	data_.emplace_back(versioned_data{std::move(new_data), ++last_version_, cursor_pos});
	current_data_++;

	crop_history();

	_has_unsaved_changes = true;

	document_changed_signal();
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
