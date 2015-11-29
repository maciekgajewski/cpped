#include <vector>
#include <string>

namespace cpped { namespace  document {

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

	void load_from_raw_data(const std::string& data);
	void load_from_raw_data(std::vector<char> data);
	void load_from_file(const std::string& path);

	int get_line_count() const { return lines.size(); }
	int left_bar_width() const;

	// TODO temp
	int line_length(int) { return 0; }
	int left_bar_width() { return 0; }

	document_line& get_line(unsigned index) { return lines.at(index); }

private:

	void parse_raw_buffer();

	std::vector<char> raw_data;
	std::vector<document_line> lines;

};



}}
