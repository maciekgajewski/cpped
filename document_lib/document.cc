#include "document.hh"
#include "clang.hh"

#include <boost/tokenizer.hpp>

#include <fstream>
#include <iostream>

namespace cpped { namespace  document {

void document::load_from_raw_data(const std::string& data, const std::string& fake_path)
{
	file_name = fake_path;
	raw_data.assign(data.begin(), data.end());
	parse_raw_buffer();
}

void document::load_from_file(const std::string& path)
{
	file_name = path;
	std::fstream f(path, std::ios_base::in);
	if (f.fail())
	{
		throw std::runtime_error("Error opening file");
	}

	raw_data.clear();

	const unsigned bufsize = 4096;
	char buf[bufsize];
	auto c = f.readsome(buf, bufsize);
	while( c > 0)
	{
		raw_data.insert(raw_data.end(), buf, buf+c);
		c = f.readsome(buf, bufsize);
	}

	parse_raw_buffer();
}

void document::parse_language()
{
	clang::index index(0, 0);
	clang::translation_unit tu(index, file_name.c_str(), raw_data.data(), raw_data.size());

	// lets try tokenizing line by line
	// TODO or should it be the entire file?
	clang::file file = tu.get_file(file_name);
	unsigned line_number = 1;
	for(const document_line& line : lines)
	{
		clang::source_location line_begin = tu.get_location(file, line_number, 1);
		clang::source_location line_end = tu.get_location(file, line_number, line.get_length());

		clang::source_range range(line_begin, line_end);
		clang::token_list tokens = tu.tokenize(range);

		std::cout << "line: " << line_number << " '" << line.to_string() << "', tokens: " << tokens.size() << std::endl;
		for(const clang::token& token : tokens)
		{
			std::cout << "  - " << token.get_kind_name() << std::endl;
		}
		line_number++;
	}
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
	lines.clear();

	// tokenize by lines
	boost::char_separator<char> endline_sep("\n", "", boost::keep_empty_tokens);
	boost::tokenizer<decltype(endline_sep), std::vector<char>::const_iterator, range> tokens(raw_data, endline_sep);

	for(const range& token : tokens)
	{
		lines.emplace_back(const_cast<char*>(&*token.begin), token.end - token.begin);
		assert(lines.back().get_data() >= raw_data.data() && lines.back().get_data() <= raw_data.data()+raw_data.size());
	}
}


}}
