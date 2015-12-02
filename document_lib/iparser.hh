#pragma once

namespace cpped { namespace  document {

class document;

class iparser
{
public:

	virtual ~iparser() {}
	virtual void parse(document& doc) = 0;
};

}}
