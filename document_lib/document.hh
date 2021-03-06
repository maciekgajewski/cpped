#pragma once

#include "document_data.hh"

#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

#include <vector>
#include <string>
#include <ostream>
#include <cassert>
#include <memory>
#include <chrono>
#include <list>

namespace cpped { namespace  document {

class document;

// read-only view on all the data about document line
class document_line
{
public:

	document_line(const line_data& line) : line_data_(line) {}

	unsigned get_length() const { return line_data_.get_length(); }
	const char* get_data() const { return &*line_data_.get_start(); }

	std::string to_string() const { return line_data_.to_string(); }

	// Calls 'fun' for each token of the line. If section of the line is not covered by a token, empty token is used
	template<typename FUN>
	void for_each_token(FUN fun) const
	{
		line_data_.for_each_token(fun);
	}

	const std::vector<line_token>& get_tokens() const { return line_data_.get_tokens();} // for testing

private:

	const line_data& line_data_;
};

// interface for editing the document
class document_edit
{
public:

	document_edit(document& doc);

	// insert string at location. Returns the position of the end of inserted text
	document_position insert(document_position pos, const std::string& text);

	// removes indicated range
	void remove(document_range r);
	// removes characters before the positon, returns the position of the begining of the removed range
	document_position remove_before(document_position pos, unsigned count);
	// removes characters after the positon, returns the position of the end of the removed range
	document_position remove_after(document_position pos, unsigned count);


	// commits the edit, together with current cursor position
	void commit(const document_position& cursor_pos);

private:

	document& document_;
	document_data* current_data_;
	std::unique_ptr<document_data> changed_data_;
};

class document
{
public:

	boost::signals2::signal<void()> document_changed_signal;
	boost::signals2::signal<void()> tokens_updated_signal;

	document();
	~document();
	document(document&&) = default;

	void load_from_raw_data(const std::string& data, const boost::filesystem::path& path, bool new_file = false);
	void load_from_raw_data(const std::string& data, const boost::filesystem::path& path, const token_data& tokens, bool new_file);
	void load_from_file(const boost::filesystem::path& path);

	unsigned get_line_count() const { return current_data_->data->get_line_count(); }

	unsigned line_length(unsigned index)
	{
		return get_line(index).get_length();
	}

	document_line get_line(unsigned index) const { return document_line(current_data_->data->get_line(index)); }

	// iterates over no more than 'count' lines in range, starting from first_line
	template<typename FUN>
	void for_lines(unsigned first_line, unsigned max_count, FUN f) const
	{
		current_data_->data->for_lines(first_line, max_count,
			[&](const line_data& ld)
			{
				f(document_line(ld));
			});
	}

	const std::vector<char>& get_raw_data() const { return current_data_->data->get_raw_data(); }
	const boost::filesystem::path& get_path() const { return path_; }
	void set_path(const boost::filesystem::path& path);

	std::string to_string() const; // mostly for testing

	document_data& get_data() { return *current_data_->data; } // for tests
	const document_data& get_data() const { return *current_data_->data; } // for tests
	std::uint64_t get_current_version() const { return current_data_->version; }

	document_position get_last_position() const { return current_data_->data->get_last_position(); }

	bool has_unsaved_changes() const { return current_data_->has_unsaved_changes; }
	void set_saved(std::uint64_t version);

	void set_tokens(std::uint64_t version, const token_data& tokens);

	const std::vector<diagnostic_message>& get_diagnostics() const { return current_data_->data->get_diagnostics(); }

	document_edit edit() { return document_edit(*this); }
	void commit_change(std::unique_ptr<document_data>&& new_data, const document_position& cursor_pos);
	std::string get_range_content(const document_range& rng) const { return get_data().get_range_content(rng); }

private:

	struct versioned_data
	{
		std::unique_ptr<document_data> data;
		std::uint64_t version;
		document_position cursor_position;
		bool has_unsaved_changes;
	};

	void erase_redo(); // erases any data ahead of current_data_
	void crop_history(); // reduces the number of items before current_data_ to predefined value

	std::list<versioned_data> data_;
	std::list<versioned_data>::iterator current_data_;

	boost::filesystem::path path_;

	std::uint64_t last_version_ = 0;
};

}}
