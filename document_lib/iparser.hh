#pragma once

#include <string>

namespace cpped { namespace  document {

class document_data;

class iparser
{
public:

	virtual ~iparser() {}
	virtual void parse(document_data& data, const std::string& file_name) = 0;
};

}}
