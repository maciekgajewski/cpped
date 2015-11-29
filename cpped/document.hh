#pragma once

#include <vector>
#include <string>

namespace cpped {

class ncurses_window;

class document_line
{
public:
	document_line(char* b, unsigned l) : begin(b), length(l) {}

	unsigned get_length() const { return length; }

private:
	char* begin;
	unsigned length;
};

class document
{
public:

	void load_from_file(const std::string& path);

	int get_line_count() const { return lines.size(); }
	int left_bar_width() const;
	int line_length(int) const { return 0; } // dummy

	document_line& get_line(unsigned index) { return lines.at(index); }

	void render(ncurses_window& win, int first_line, int first_column, int lines, int columns);

private:

	int left_bar_digits = 1;

	std::vector<char> raw_data;
	std::vector<document_line> lines;

};

}
