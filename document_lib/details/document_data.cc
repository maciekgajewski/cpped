#include "document_data.hh"

#include <boost/tokenizer.hpp>

#include <fstream>

namespace cpped { namespace  document { namespace details {


void line_data::push_back_token(const line_token& t)
{
	assert(t.end <= length);
	assert(t.begin >= (tokens.empty() ? 0u : tokens.back().end));
	tokens_.push_back(t);
}

void document_data::load_from_file(const std::string& path)
{
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
	raw_data_.assign(data.begin(), data.end());
	create_lines();
}

void document_data::copy_inserting(const document_data& source, position pos, const std::string& text)
{
	// TODO
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

/* TODO use?
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
*/

}}}
