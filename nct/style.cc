#include "style.hh"

#include "ncurses_env.hh"

namespace nct {

int style::to_attr() const
{
	return ncurses_env::get_current()->style_to_attr(*this);
}

}
