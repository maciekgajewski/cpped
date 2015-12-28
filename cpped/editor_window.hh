#pragma once

#include "editor.hh"

#include "navigator_widget.hh"

#include "nct/event_window.hh"

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

	void render(document::document& doc, unsigned first_column, unsigned first_line, unsigned tab_width);
	void update_status_info(const status_info& info);
	void refresh_cursor(int wy, int wx);

	unsigned get_workspace_width() const;
	unsigned get_workspace_height() const;

	void open_file(const boost::filesystem::path& file);

private:

	// rendering
	unsigned render_text(nct::ncurses_window& window, attr_t attr, unsigned tab_width, unsigned first_column, unsigned phys_column, const char* begin, const char* end); // returns last physical column
	void put_visual_tab(nct::ncurses_window& window);

	unsigned left_margin_width_ = 0; // calculated when rendering
	unsigned top_margin_ = 2;
	unsigned bottom_margin_ = 1;

	// settings
	bool visualise_tabs_ = true;

	project& project_;
	style_manager& styles_;
	editor editor_;
	navigator_widget navigator_;
};

}
