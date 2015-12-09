#pragma once

#include "../line_token.hh"

#include <vector>

namespace cpped { namespace  document { namespace details {

using data_type = std::vector<char>;

class line_data
{
public:

	line_data(data_type::const_iterator begin, unsigned length) : begin_(begin), length_(length) {}

	void clear_tokens() { tokens_.clear(); }
	void push_back_token(const line_token& t);

private:
	data_type::const_iterator begin_;
	unsigned length_;

	std::vector<line_token> tokens_;
};

class document_data
{
public:

	document_data(const document_data&) = delete; // make sure there is no deep-copying
	document_data(document_data&&) = default;

	// initializing functions
	void load_from_file(const std::string& path);
	void load_from_raw_data(const std::string& data);
	void copy_inserting(const document_data& source, position pos, const std::string& text);

	std::string to_string() const { return std::string(data_.begin(), data_.end()); }

private:

	void create_lines(); // creates token-less lines for freshly loaded buffer

	// Immutable data. Never modified, except when on construction
	data_type raw_data_;
	std::vector<line_data> lines_;
};

}}}
