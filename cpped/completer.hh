#pragma once

#include "project.hh"

#include "nct/event_window.hh"

namespace cpped {

// code-completion widget
class completer final : public nct::event_window
{
public:

	completer(project& pr, nct::event_dispatcher& ed, nct::event_window* parent);

	// activates on user request in specific point of a document.
	// Requests completion sychronously
	void activate(const document::document& doc, const document::document_position& cursor_pos);

private:

	static bool is_valid_identifier(const char* s, unsigned len);

	project& project_;
};

}
