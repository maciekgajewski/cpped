#include "document.hh"
#include "clang.hh"

#include <boost/tokenizer.hpp>

#include <fstream>
#include <iostream>

namespace cpped { namespace  document {

std::ostream& operator << (std::ostream& s, token_type tt)
{
	switch(tt)
	{
		case token_type::none : return s << "none";
		case token_type::keyword : return s << "keyword";
		case token_type::literal : return s << "literal";
		case token_type::preprocessor : return s << "preprocessor";
		case token_type::type : return s << "type";
		case token_type::comment : return s << "comment";
		case token_type::max_tokens : return s << "max_tokens";
	}
	return s << "?";
}

std::ostream& operator << (std::ostream& s, const line_token& t)
{
	return s << "token{b="<< t.begin << ", e=" << t.end << ", t=" << t.type << "}";
}


document::document()
{
	// add one empty line
	lines_.emplace_back(*this, nullptr, 0);
}

document::~document()
{
}

void document::load_from_raw_data(const std::string& data, const std::string& fake_path, std::unique_ptr<iparser>&& p)
{
	file_name = fake_path;
	raw_data_.assign(data.begin(), data.end());
	parse_raw_buffer();

	parser = std::move(p);
}

void document::load_from_file(const std::string& path, std::unique_ptr<iparser>&& p)
{
	file_name = path;
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

	parse_raw_buffer();

	parser = std::move(p);
}

void document::insert(unsigned line, unsigned col, const std::string& text)
{
	// TODO rewrtie
	for(char c : text)
	{
		auto& doc_line = get_line(line);
		doc_line.insert(col, c);
		if (c == '\n')
		{
			col = 0;
			line++;
		}
		else
		{
			col++;
		}
	}
}

void document::parse_language()
{
	if (parser)
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		parser->parse(*this);
		auto end_time = std::chrono::high_resolution_clock::now();

		last_parse_time = end_time - start_time;
	}

}

void document::insert(const char* position, char c)
{
	const char* old_data = nullptr;
	bool reallocated = false;
	if (raw_data_.size() + 1 > raw_data_.capacity())
	{
		// this will reallocate. Preserve original data ptr
		old_data = raw_data_.data();
		reallocated = true;
	}
	raw_data_.insert(raw_data_.begin() + (position-raw_data_.data()), c);

	if (reallocated)
	{
		// old_data is invalid now, do not dereference!
		for(document_line& line : lines_)
		{
			line.refresh_position(old_data, raw_data_.data());
		}
	}
}

void document::shift_lines(document_line* after, unsigned shift)
{
	auto it = lines_.begin() + (after-lines_.data());
	for(it++; it != lines_.end(); it++)
	{
		it->shift(shift);
	}

}

void document::insert_line(document_line* after, document_line&& new_line)
{
	auto it = lines_.begin() + (after-lines_.data()) + 1;
	lines_.insert(it, std::move(new_line));
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

void document::parse_raw_buffer()
{
	lines_.clear();

	// tokenize by lines
	boost::char_separator<char> endline_sep("\n", "", boost::keep_empty_tokens);
	boost::tokenizer<decltype(endline_sep), std::vector<char>::const_iterator, range> tokens(raw_data_, endline_sep);

	for(const range& token : tokens)
	{
		lines_.emplace_back(*this, const_cast<char*>(&*token.begin), token.end - token.begin);
		assert(lines_.back().get_data() >= raw_data_.data() && lines_.back().get_data() <= raw_data_.data()+raw_data_.size());
	}
}

void document_line::push_back_token(const line_token& t)
{
	assert(t.end <= length);
	assert(t.begin >= (tokens.empty() ? 0u : tokens.back().end));
	tokens.push_back(t);
}

void document_line::insert(unsigned position, char c)
{
	assert(position <= get_length());

	// modify the underlying data buffer
	parent.insert(begin+position, c);

	if (c == '\n')
	{
		document_line new_line(parent, begin+position+1, length - position);

		length = position;
		// move tokens to the next line
		auto first_to_move = std::find_if(tokens.begin(), tokens.end(),
			[&](const line_token& t)
			{
				return t.end > position;
			});

		if (first_to_move != tokens.end())
		{
			unsigned tokens_to_move = tokens.size() - (tokens.begin()-first_to_move);
			if (first_to_move->begin < position)
			{
				tokens_to_move++;
			}
			new_line.tokens.reserve(tokens_to_move);
			if (first_to_move->begin < position)
			{
				new_line.tokens.push_back(line_token{0, first_to_move->end-position, first_to_move->type});
				first_to_move->end = position;
				first_to_move ++;
			}

			std::transform(first_to_move, tokens.end(), std::back_inserter(new_line.tokens),
				[&](const line_token& token)
				{
					return line_token{token.begin-position, token.end-position, token.type};
				});

			tokens.erase(first_to_move, tokens.end());
		}

		parent.shift_lines(this, 1);
		parent.insert_line(this, std::move(new_line)); // last statement, as this may delete this
	}
	else
	{
		// modify tokens
		for(line_token& token : tokens)
		{
			if (token.end > position)
			{
				token.end++;
				if (token.begin > position)
					token.begin++;
			}
		}
		length++;
		parent.shift_lines(this, 1);
	}

}


}}
