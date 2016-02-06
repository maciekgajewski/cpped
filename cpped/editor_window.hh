#pragma once

#include "editor.hh"
#include "completer_widget.hh"

#include "nct/event_window.hh"
#include "nct/status_message.hh"

#include <boost/filesystem.hpp>

#include <chrono>

namespace nct {
class ncurses_window;
}

namespace cpped {

class style_manager;
class edited_file;

namespace document{
	class document;
}

class editor_window final : public nct::event_window
{
public:

	editor_window(project& pr, nct::window_manager& ed, style_manager& sm, edited_file& f, nct::event_window* parent = nullptr);

	unsigned on_sequence(const std::string& s) override;
	bool on_special_key(int key_code, const char* key_name) override;
	void on_mouse(const MEVENT& event) override;
	void on_shown() override;
	void on_resized() override;

	void open_file(const boost::filesystem::path& path);
	void open_file(edited_file& file);

	// inward-facing API, used by editor

	void request_full_render() { request_redraw(); }

	unsigned get_workspace_width() const;
	unsigned get_workspace_height() const;

	project& get_project() const { return project_; }
	void set_status(const std::string& s) { status_provider_.set_status(s); }

	void save_as(const boost::filesystem::path& path);
	void save();

private:

	void render(nct::ncurses_window& surface) override;
	void render_status_info(nct::ncurses_window& surface, const editor::status_info& info);
	void update_cursor(const nct::position& pos);

	unsigned left_margin_width_ = 0; // calculated when rendering
	unsigned top_margin_ = 0;
	unsigned bottom_margin_ = 1;

	project& project_;
	style_manager& styles_;
	editor editor_;
	completer_widget completer_;
	nct::position cursor_pos_;
	nct::status_provider status_provider_;
};

}
