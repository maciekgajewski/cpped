#include "iparser.hh"
#include "clang.hh"

namespace cpped { namespace document {

class document_data;

class cpp_parser : public iparser
{
public:

	cpp_parser(clang::translation_unit& tu);
	void parse(document_data& data, const std::string& file_name) override;

private:

	clang::translation_unit& translation_unit_;
};

}}
