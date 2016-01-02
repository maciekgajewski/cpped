#pragma once

#include "document_lib/document.hh"

#include "nct/style.hh"

#include <array>

namespace cpped
{

class style_manager
{
public:
	style_manager();

	nct::style get_style_for_token(document::token_type tt) { return style_for_token[int(tt)]; }

	nct::style plain;
	nct::style line_numbers;
	nct::style visual_tab;
	nct::style status;

private:

	std::array<nct::style, int(document::token_type::max_tokens)> style_for_token;
};

}
