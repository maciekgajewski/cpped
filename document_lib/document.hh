#pragma once

#include "iparser.hh"

#include "document_data.hh"

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

class document
{
public:

	document();
	~document();

	void load_from_raw_data(const std::string& data, const std::string& fake_path, std::unique_ptr<iparser>&& parser_ = nullptr);
	void load_from_file(const std::string& path, std::unique_ptr<iparser>&& parser_ = nullptr);

	unsigned get_line_count() const { return current_data_->get_line_count(); }

	unsigned line_length(unsigned index)
	{
		return get_line(index).get_length();
	}

	document_line get_line(unsigned index) const { return document_line(current_data_->get_line(index)); }

	// insert string at location. Returns the position of the end of inserted text
	position insert(position pos, const std::string& text);

	// removes indicated range
	void remove(range r);
	// removes characters before the positon, returns the position of the begining of the removed range
	position remove_before(position pos, unsigned count);
	// removes characters after the positon, returns the position of the end of the removed range
	position remove_after(position pos, unsigned count);


	// iterates over no more than 'count' lines in range, starting from first_line
	template<typename FUN>
	void for_lines(unsigned first_line, unsigned max_count, FUN f)
	{
		current_data_->for_lines(first_line, max_count,
			[&](const line_data& ld)
			{
				f(document_line(ld));
			});
	}

	void parse_language();

	const std::vector<char>& get_raw_data() const { return current_data_->get_raw_data(); }
	const std::string& get_file_name() const { return file_name_; }

	std::string to_string() const; // mostly for testing

	std::chrono::high_resolution_clock::duration get_last_parse_time() const { return last_parse_time_; }

	document_data& get_data() { return *current_data_; } // for tests

	position get_last_position() const { return current_data_->get_last_position(); }

private:

	void erase_redo(); // erases any data ahead of current_data_
	void crop_history(); // reduces the number of items before current_data_ to predefined value

	std::list<document_data> data_;
	std::list<document_data>::iterator current_data_;

	std::string file_name_;

	std::unique_ptr<iparser> parser_;
	std::chrono::high_resolution_clock::duration last_parse_time_;
};

}}
