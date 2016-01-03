#pragma once

#include "line_data.hh"

#include <vector>

namespace cpped { namespace  document {


class document_data
{
public:

	document_data() = default;
	document_data(const document_data&) = delete; // make sure there is no deep-copying
	document_data(document_data&&) = default;

	// initializing functions
	void init_empty();
	void load_from_file(const std::string& path);
	void load_from_raw_data(const std::string& data);
	document_position copy_inserting(const document_data& source, document_position pos, const std::string& text);
	void copy_removing(const document_data& source, document_range to_remove);

	std::string to_string() const { return std::string(raw_data_.begin(), raw_data_.end()); }

	bool empty() const { return raw_data_.empty() && lines_.empty(); }

	unsigned get_line_count() const { return lines_.size(); }
	const line_data& get_line(unsigned index) const { return lines_.at(index); }
	line_data& get_line(unsigned index) { return lines_.at(index); }

	// iterates over no more than 'count' lines in range, starting from first_line
	template<typename FUN>
	void for_lines(unsigned first_line, unsigned max_count, FUN f)
	{
		if (first_line < lines_.size())
		{
			unsigned count = std::min<unsigned>(lines_.size()-first_line, max_count);
			auto it = lines_.begin() + first_line;
			for(unsigned i = 0; i < count; ++i, ++it)
			{
				f(*it);
			}
		}
	}

	const data_type& get_raw_data() const { return raw_data_; }

	// returns position shifted by number of characters back
	document_position shift_back(document_position p, unsigned shift);
	// returns position shifted by number of characters forward
	document_position shift_forward(document_position p, unsigned shift);

	document_position get_last_position() const { return document_position{unsigned(lines_.size()-1), unsigned(lines_.back().get_length())}; }

	// replaces all the existing tokens
	void set_tokens(const token_data& tokens);

	const line_token* get_token_at(const document_position& pos) const;

	// returns token immediately before the position (pair: line, token)
	std::pair<unsigned, const line_token*> get_token_before(const document_position& pos) const;

	const std::vector<diagnostic_message>& get_diagnostics() const { return diagnostics_; }

	std::string get_range_content(const document_range& rng) const;

private:

	data_type::const_iterator positon_to_offset(document_position pos) const;

	void create_lines(); // creates token-less lines for freshly loaded buffer

	// Immutable data. Never modified, except when on construction.
	// It must be continous, char*, utf-8 data, so it can be directly consumed bu libclang
	data_type raw_data_;
	std::vector<line_data> lines_;
	std::vector<diagnostic_message> diagnostics_;
};

}}
