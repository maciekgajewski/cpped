#pragma once

#include "document_lib/document_data.hh"

namespace cpped {

// Editor state preserved on per-file bases when file is "open" but not displayed
struct editor_state
{
	document::document_position cursor_position;
	unsigned first_line;
};

}
