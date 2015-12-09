#include "iparser.hh"
#include "clang.hh"

namespace cpped { namespace document {

class document_data;

class cpp_parser : public iparser
{
public:

	cpp_parser();
	void parse(document_data& data, const std::string& file_name) final override;

private:

	clang::index index_;
	clang::translation_unit translation_unit_;
};

}}
