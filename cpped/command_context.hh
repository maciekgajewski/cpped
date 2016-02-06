#pragma once

#include <nct/list_widget.hh>

#include <boost/filesystem.hpp>


namespace cpped {

class project;

// Tool trough which a command sees the system
class command_context
{
public:

	using hint_list = std::vector<nct::list_widget::list_item>;

	// Open file in current editor, finish command entry
	virtual void open_file(const boost::filesystem::path& path) = 0;

	// Saves current file as
	virtual void save_as(const boost::filesystem::path& path) = 0;

	/// Displays hint widget, shifter 'position' characters to the left from the beginnig
	/// of the line edit
	virtual void show_hints(unsigned position, const hint_list& items) = 0;

	virtual nct::list_widget::list_item* get_current_item() = 0;

	// Changes editor text. Warning! May call on_text_changed iemdiately, destroying the command!
	virtual void set_text(const std::string& t) = 0;

	virtual std::string get_text() const = 0;
	virtual const project& get_project() const = 0;

private:

};

}
