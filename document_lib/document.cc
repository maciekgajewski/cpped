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

	clang::source_file file = tu.get_file(file_name);

	// full file tokenizer. doing it line-by-line is not a good approach
	clang::source_location file_begin = tu.get_location(file, 1, 1);
	clang::source_location file_end = tu.get_location(file, lines.size() + 1, lines.back().get_length());
	clang::source_range range(file_begin, file_end);
	clang::token_list tokens = tu.tokenize(range);
	tokens.annotate_tokens();

	std::cout << "entire file: tokens=" << tokens.size() << std::endl;
	unsigned line_number = 0;
	for(const clang::token& token : tokens)
	{
		clang::source_location location = tokens.get_token_location(token);
		clang::source_location::info info = location.get_location_info();
		if (info.line != line_number)
		{
			line_number = info.line;
			std::cout << std::endl;
			std::cout << "line: " << line_number << " '" << lines[line_number-1].to_string() << std::endl;
		}
		if (token.has_associated_cursor())
		{
			clang::string type = token.get_associated_cursor().get_kind_as_string();
			std::cout << token.get_kind_name() << " (" << type << "), ";
		}
		else
		{
			std::cout << token.get_kind_name() << ", ";
		}
	}
	std::cout << std::endl;
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
