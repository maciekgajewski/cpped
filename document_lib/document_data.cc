#include "document_data.hh"

#include <boost/tokenizer.hpp>

#include <fstream>

namespace cpped { namespace  document {


void document_data::init_empty()
{
	lines_.emplace_back(raw_data_.begin(), 0);
}

void document_data::load_from_file(const std::string& path)
{
	assert(empty());

	std::fstream f(path, std::ios_base::in);
	if (f.fail())
	{
		throw std::runtime_error("Error opening file");
	}

	raw_data_.clear();

	const unsigned bufsize = 4096;
	char buf[bufsize];
	auto c = f.readsome(buf, bufsize);
	while( c > 0)
	{
		raw_data_.insert(raw_data_.end(), buf, buf+c);
		c = f.readsome(buf, bufsize);
	}

	create_lines();
}

void document_data::load_from_raw_data(const std::string& data)
{
	assert(empty());

	raw_data_.assign(data.begin(), data.end());
	create_lines();
}

document_position document_data::copy_inserting(const document_data& source, document_position pos, const std::string& text)
{
	assert(empty());
	assert(pos.line < source.lines_.size());

	document_position final_pos = pos;

	// copy data, inserting the new chunk
	raw_data_.reserve(source.raw_data_.size() + text.length());

	auto insert_pos = source.positon_to_offset(pos);
	raw_data_.insert(raw_data_.end(), source.raw_data_.begin(), insert_pos);
	raw_data_.insert(raw_data_.end(), text.begin(), text.end());
	raw_data_.insert(raw_data_.end(), insert_pos, source.raw_data_.end());

	assert(raw_data_.size() == source.raw_data_.size() + text.length() && "incorrect data size after inserting");

	// calculate number of lines of the inserted text
	unsigned inserted_lines = std::count(text.begin(), text.end(), '\n');
	lines_.reserve(source.lines_.size() + inserted_lines);

	// copy all lines from [0, pos.line) without changes
	auto modified_line_it = source.lines_.begin() + pos.line;
	auto start = raw_data_.begin();
	for (auto it = source.lines_.begin(); it != modified_line_it; ++it)
	{
		lines_.emplace_back(start, it->get_length());
		lines_.back().copy_tokens(*it);
		start += it->get_length() + 1;
	}

	// modify the line into which the text is inserted
	if (inserted_lines == 0)
	{
		unsigned len = modified_line_it->get_length() + text.size();
		lines_.emplace_back(start, len);
		lines_.back().copy_and_insert(*modified_line_it, pos.column, text);
		start += len + 1;
		final_pos.column += text.size();
	}
	else
	{
		// split the line into two, insert lines
		auto begin = text.begin();
		auto npos = std::find(text.begin(), text.end(), '\n');
		unsigned len = pos.column + npos - begin;
		lines_.emplace_back(start, len);
		lines_.back().copy_truncated(*modified_line_it, pos.column, npos - begin);
		start += len +1;

		while(npos != text.end())
		{
			begin = npos + 1;
			npos = std::find(begin, text.end(), '\n');

			if (npos == text.end())
			{
				// last line
				len = npos - begin + modified_line_it->get_length() - pos.column;
				lines_.emplace_back(start, len);
				lines_.back().copy_remainder(*modified_line_it, pos.column, npos - begin);
				final_pos.column = npos - begin;
				final_pos.line += inserted_lines;
			}
			else
			{
				len = npos - begin;
				lines_.emplace_back(start, len);
			}
			start += len + 1;
		}

	}

	// copy remaining lines verbatim
	for(modified_line_it++; modified_line_it != source.lines_.end(); modified_line_it++)
	{
		lines_.emplace_back(start, modified_line_it->get_length());
		lines_.back().copy_tokens(*modified_line_it);
		start += modified_line_it->get_length() + 1;
	}

	start--; // last line doesn't have endl at the end

	assert(start == raw_data_.end() && "inconsisted line lengths after insertion");

	return final_pos;
}

void document_data::copy_removing(const document_data& source, document_range to_remove)
{
	assert(empty() && "this is to be called immediately after initialization");
	assert(to_remove.start.line < source.lines_.size());
	assert(to_remove.end.line < source.lines_.size());

	// copy data skipping the removed chunk
	auto start_offset = source.positon_to_offset(to_remove.start);
	auto end_offset = source.positon_to_offset(to_remove.end);
	int removed_chars = end_offset - start_offset;

	raw_data_.reserve(source.raw_data_.size() - removed_chars);
	raw_data_.insert(raw_data_.end(), source.raw_data_.begin(), start_offset);
	raw_data_.insert(raw_data_.end(), end_offset, source.raw_data_.end());
	assert(raw_data_.size() == source.raw_data_.size() - removed_chars);

	// copy lines, preserving tokens in partially removed lines
	int lines_removed = to_remove.end.line - to_remove.start.line;

	lines_.reserve(source.lines_.size() - lines_removed);

	// copy all lines from [0, pos.line) without changes
	auto modified_line_it = source.lines_.begin() + to_remove.start.line;
	auto start = raw_data_.begin();
	for (auto it = source.lines_.begin(); it != modified_line_it; ++it)
	{
		lines_.emplace_back(start, it->get_length());
		lines_.back().copy_tokens(*it);
		start += it->get_length() + 1;
	}

	if (lines_removed == 0)
	{
		// copy the one changed line, removing range
		unsigned len = modified_line_it->get_length() - (to_remove.end.column - to_remove.start.column);
		lines_.emplace_back(start, len);
		lines_.back().copy_and_remove(*modified_line_it, to_remove.start.column, to_remove.end.column);
		start += len + 1;
	}
	else
	{
		// find the first and the last changed line
		// create line consiting of the beginnig of the first and the tail of the last one
		auto first_changed = modified_line_it;
		auto last_changed = modified_line_it + lines_removed;

		unsigned len = to_remove.start.column // this much of the first line
			+ last_changed->get_length() - to_remove.end.column; // this much of the last line
		lines_.emplace_back(start, len);
		lines_.back().copy_from_two_clipped(*first_changed, to_remove.start.column, *last_changed, to_remove.end.column);
		start += len + 1;

		modified_line_it = last_changed;
	}

	for(modified_line_it++; modified_line_it != source.lines_.end(); modified_line_it++)
	{
		lines_.emplace_back(start, modified_line_it->get_length());
		lines_.back().copy_tokens(*modified_line_it);
		start += modified_line_it->get_length() + 1;
	}

	start--; // last line doesn't have endl at the end

	assert(start == raw_data_.end() && "inconsisted line lengths after removal");
}

document_position document_data::shift_back(document_position p, unsigned shift)
{
	assert(p.line < lines_.size());
	assert(p.column <= lines_[p.line].get_length());

	while(shift > 0)
	{
		if (p.column >= shift)
		{
			p.column -= shift;
			shift = 0;
		}
		else
		{
			// move one line up, if possible
			if (p.line == 0)
				break;

			shift -= p.column+1;
			p.line--;
			p.column = lines_[p.line].get_length();
		}
	}
	return p;
}

document_position document_data::shift_forward(document_position p, unsigned shift)
{
	assert(p.line < lines_.size());
	assert(p.column <= lines_[p.line].get_length());
	while(shift > 0)
	{
		if (lines_[p.line].get_length() - p.column >= shift)
		{
			p.column += shift;
			shift = 0;
		}
		else
		{
			// move one line down, if possible
			if (p.line == lines_.size()-1)
				break;

			shift -= lines_[p.line].get_length() - p.column + 1;
			p.line++;
			p.column = 0;
		}
	}
	return p;
}

data_type::const_iterator document_data::positon_to_offset(document_position pos) const
{
	assert(pos.line < lines_.size());
	assert(pos.column <= lines_[pos.line].get_length());

	return lines_[pos.line].get_start() + pos.column;
}

void document_data::create_lines()
{
	struct range
	{
		std::vector<char>::const_iterator begin;
		std::vector<char>::const_iterator end;

		void assign(std::vector<char>::const_iterator b, std::vector<char>::const_iterator e)
		{
			begin = b;
			end = e;
		}
	};

	lines_.clear();

	// tokenize by lines
	boost::char_separator<char> endline_sep("\n", "", boost::keep_empty_tokens);
	boost::tokenizer<decltype(endline_sep), std::vector<char>::const_iterator, range> tokens(raw_data_, endline_sep);

	for(const range& token : tokens)
	{
		lines_.emplace_back(token.begin, token.end - token.begin);
	}
}

}}
