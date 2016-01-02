#pragma once

#include <boost/container/flat_map.hpp>

namespace nct {

struct style
{
	int bgcolor;
	int fgcolor;
	int flags = 0;

	int to_attr() const;
};

class color_palette
{
public:

	int get_pair_for_colors(int bg, int fg);

private:
	boost::container::flat_map<std::pair<int, int>, int> color_pairs;
};

}
