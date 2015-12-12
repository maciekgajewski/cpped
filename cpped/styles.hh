#pragma once

#include "document_lib/document.hh"

#include "nct/event_dispatcher.hh"

#include <array>

namespace cpped
{

class style_manager
{
public:
	style_manager(nct::color_palette& pal);

	int get_attr_for_token(document::token_type tt) { return attr_for_token[int(tt)]; }

	int line_numbers;
	int visual_tab;
	int status;

	nct::color_palette& palette;

private:

	std::array<int, int(document::token_type::max_tokens)> attr_for_token;
};

}
