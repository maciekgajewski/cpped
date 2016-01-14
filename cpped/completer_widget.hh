#pragma once

#include "project.hh"

#include "nct/event_window.hh"
#include "nct/list_widget.hh"

#include <boost/signals2.hpp>

namespace cpped {

// code-completion widget
class completer_widget final : public nct::event_window
{
public:

	boost::signals2::signal<void()> completion_cancelled_signal;
	boost::signals2::signal<void(const document::document_position& pos, unsigned len, const std::string&)> completion_signal;

	completer_widget(project& pr, nct::window_manager& ed, nct::event_window* parent);

	// activates on user request in specific point of a document.
	// Requests completion sychronously
	void activate(
		const document::document& doc,
		const document::document_position& cursor_pos,
		const nct::position& screen_pos);

private:

	unsigned on_sequence(const std::string& s) override;
	 bool on_special_key(int key_code, const char* key_name) override;

	void cancel();
	void complete();

	static bool is_valid_identifier(const char* s, unsigned len);

	project& project_;
	std::string filter_;
	document::document_position start_position_;
	boost::optional<nct::list_widget> list_;
};

}
