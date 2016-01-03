#include "clipboard.hh"

#include <stdexcept>

namespace cpped
{

clipboard* clipboard::instance_ = nullptr;

clipboard::clipboard()
{
	if (instance_)
		throw std::logic_error("There can be only one clipboard");
	instance_ = this;
}

clipboard::~clipboard()
{
	assert(instance_ == this);
	instance_ = nullptr;
}

}
