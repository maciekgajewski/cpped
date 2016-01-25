#pragma once

#include <nct/list_widget.hh>
#include <nct/line_edit.hh>

#include <string>
#include <memory>
#include <vector>

namespace cpped {

struct command_context
{
	nct::window_manager& wm;
	nct::position editor_pos;
	unsigned editor_width;
	nct::list_widget* hint_list;
	nct::line_edit* command_editor;
};

// command and command factory interfaces
class icommand
{
public:

	virtual void on_text_changed(const std::string& text) = 0;
	virtual bool on_enter_pressed() = 0;
};

class icommand_factory
{
public:
	virtual std::unique_ptr<icommand> create_command(const command_context& ctx, const std::string& prefix) const = 0;
};

struct command_entry
{
	std::string name;
	std::string help;
	icommand_factory* factory;
};

using command_factory_list = std::vector<command_entry>;



// root command
std::unique_ptr<icommand> make_root_command(const command_context& ctx);

}
