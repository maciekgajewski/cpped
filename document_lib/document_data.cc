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

position document_data::copy_inserting(const document_data& source, position pos, const std::string& text)
{
	assert(empty());
	assert(pos.line < source.lines_.size());

	position final_pos = pos;

	// calculate number of lines of the inserted text
	unsigned inserted_lines = std::count(text.begin(), text.end(), '\n');


	// copy data, inserting the new chunk
	raw_data_.reserve(source.raw_data_.size() + text.length());

	auto insert_pos = source.positon_to_offset(pos);
	raw_data_.insert(raw_data_.end(), source.raw_data_.begin(), insert_pos);
	raw_data_.insert(raw_data_.end(), text.begin(), text.end());
	raw_data_.insert(raw_data_.end(), insert_pos, source.raw_data_.end());

	// copy all lines from [0, pos.line) without changes
	lines_.reserve(source.lines_.size() + inserted_lines);

	auto modified_line = source.lines_.begin() + pos.line;
	auto start = raw_data_.begin();
	for (auto it = source.lines_.begin(); it != modified_line; ++it)
	{
		lines_.emplace_back(start, it->get_length());
		lines_.back().copy_tokens(*it);
		start += it->get_length() + 1;
	}

	// modify the line into which the text is inserted
	if (inserted_lines == 0)
	{
		unsigned len = modified_line->get_length() + text.size();
		lines_.emplace_back(start, len);
		lines_.back().copy_and_insert(*modified_line, pos.column, text);
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
		lines_.back().copy_truncated(*modified_line, pos.column, npos - begin);
		start += len +1;

		while(npos != text.end())
		{
			begin = npos + 1;
			npos = std::find(begin, text.end(), '\n');

			if (npos == text.end())
			{
				// last line
				len = npos - begin + modified_line->get_length() - pos.column;
				lines_.emplace_back(start, len);
				lines_.back().copy_remainder(*modified_line, pos.column, npos - begin);
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
	for(modified_line++; modified_line != source.lines_.end(); modified_line++)
	{
		lines_.emplace_back(start, modified_line->get_length());
		lines_.back().copy_tokens(*modified_line);
		start += modified_line->get_length() + 1;
	}

	start--; // last line doesn't have endl at the end

	assert(start == raw_data_.end() && "inconsisted line lengths after insertion");

	return final_pos;
}

data_type::const_iterator document_data::positon_to_offset(position pos) const
{
	assert(pos.line < lines_.size());
	assert(pos.column <= lines_[pos.line].get_length());

	return lines_[pos.line].get_start() + pos.column;
}

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

void document_data::create_lines()
{
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
