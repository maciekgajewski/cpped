#pragma once

#include "editor.hh"
#include "completer_widget.hh"
#include "navigator_widget.hh"

#include "nct/event_window.hh"
#include "nct/status_message.hh"

#include <boost/filesystem.hpp>

#include <chrono>

namespace nct {
class ncurses_window;
}

namespace cpped {

class style_manager;

namespace document{
	class document;
}

class editor_window final : public nct::event_window
{
public:

	struct status_info
	{
		unsigned docy, docx, column; // current cursor coordinates
		std::string status_text; // extra text displayed in status line
		boost::filesystem::path file_name;
		bool unsaved; // if file has unsaved changes
	};

	editor_window(project& pr, nct::event_dispatcher& ed, style_manager& sm, nct::event_window* parent = nullptr);

	unsigned on_sequence(const std::string& s) override;
	bool on_special_key(int key_code, const char* key_name) override;
	void on_mouse(const MEVENT& event) override;
	void on_shown() override;
	void on_resized() override;

	void open_file(const boost::filesystem::path& file);

	// inward-facing API, used by editor

	void render(
		document::document& doc,
		unsigned first_column, unsigned first_line,
		const editor_settings& settings,
		const boost::optional<document::document_range>& selection);
	void update_status_info(const status_info& info);
	void refresh_cursor(int wy, int wx);

	unsigned get_workspace_width() const;
	unsigned get_workspace_height() const;

	project& get_project() const { return project_; }
	void set_status(const std::string& s) { status_provider_.set_status(s); }

private:

	unsigned left_margin_width_ = 0; // calculated when rendering
	unsigned top_margin_ = 2;
	unsigned bottom_margin_ = 1;

	project& project_;
	style_manager& styles_;
	editor editor_;
	navigator_widget navigator_;
	completer_widget completer_;
	nct::position cursor_pos_;
	nct::status_provider status_provider_;
};

}
