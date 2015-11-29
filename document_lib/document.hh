#pragma once

#include <vector>
#include <string>
namespace cpped { namespace  document {

class document_line
{
public:
	document_line(char* b, unsigned l) : begin(b), length(l) {}

	unsigned get_length() const { return length; }
	const char* get_data() const { return begin; }

	std::string to_string() const { return std::string(begin, begin+length); }

private:
	char* begin;
	unsigned length;
};

class document
{
public:

	void load_from_raw_data(const std::string& data, const std::__cxx11::string& fake_path);
	void load_from_raw_data(std::vector<char> data);
	void load_from_file(const std::string& path);

	unsigned get_line_count() const { return lines.size(); }
	int left_bar_width() const;

	unsigned line_length(unsigned index)
	{
		if (index == lines.size())
			return 0; // fake last line
		else
			return get_line(index).get_length();
	}

	document_line& get_line(unsigned index) { return lines.at(index); }

	// iterates over no more than 'count' lines in range, starting from first_line
	template<typename FUN>
	void for_lines(unsigned first_line, unsigned max_count, FUN f)
	{
		if (first_line < lines.size())
		{
			unsigned count = std::min<unsigned>(lines.size()-first_line, max_count);
			auto it = lines.begin() + first_line;
			for(unsigned i = 0; i < count; ++i, ++it)
			{
				f(*it);
			}
		}
	}

	void parse_language();

private:

	void parse_raw_buffer();

	std::vector<char> raw_data;
	std::vector<document_line> lines;
	std::string file_name;

};



}}
