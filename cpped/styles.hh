#pragma once

#include "document_lib/document.hh"

#include <boost/container/flat_map.hpp>

#include <array>

namespace cpped
{

class color_palette
{
public:

	int get_pair_for_colors(int bg, int fg);

private:
	boost::container::flat_map<std::pair<int, int>, int> color_pairs;
};

class style_manager
{
public:
	style_manager();

	int get_attr_for_token(document::token_type tt) { return attr_for_token[int(tt)]; }

	color_palette palette;

	int line_numbers;

private:


	std::array<int, int(document::token_type::max_tokens)> attr_for_token;
};

}
