#pragma once

#include "commands.hh"

namespace cpped {

// Command that has another command as a parameter. Used to group commands
class selector_command : public icommand
{
public:

	selector_command(const command_context& ctx, const std::string& prefix, const command_factory_list& subcommands);

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

	std::unique_ptr<icommand> create_command(const command_context& ctx, const std::string& prefix) const override
	{
		return std::make_unique<selector_command>(ctx, prefix, subcommands_);
	}

private:
	const command_factory_list& subcommands_;
};

}
