#pragma once

#include <nct/list_widget.hh>

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
	virtual std::unique_ptr<icommand> create_command(const command_context& ctx, unsigned shift) const = 0;
};

using command_factory_list = std::vector<
	std::pair<std::string, icommand_factory*>>;

// command that has another command as a parameter
class selector_command : public icommand
{
public:

	selector_command(const command_context& ctx, unsigned shift, const command_factory_list& subcommands);

	virtual void on_text_changed(const std::string& text);
	virtual bool on_enter_pressed();

private:

	void display_hints(const std::string& filter);

	const command_context& ctx_;
	unsigned shift_ = 0; // how many characters from the beginning of the editor the command starts
	std::string prefix_;
	std::unique_ptr<icommand> next_command_;

	const command_factory_list& subcommands_;
};

class selector_command_factory : public icommand_factory
{
public:
	selector_command_factory(const command_factory_list& subcommands)
		: subcommands_(subcommands) {}

	std::unique_ptr<icommand> create_command(const command_context& ctx, unsigned shift) const override
	{
		return std::make_unique<selector_command>(ctx, shift, subcommands_);
	}

private:
	const command_factory_list& subcommands_;
};


// root command
std::unique_ptr<icommand> make_root_command(const command_context& ctx);

}
