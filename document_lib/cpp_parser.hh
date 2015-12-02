#include "iparser.hh"
#include "clang.hh"

namespace cpped { namespace document {

class document;

class cpp_parser : public iparser
{
public:

	cpp_parser();
	void parse(document& doc) final override;

private:

	clang::index index;
	clang::translation_unit tu;
};

}}
