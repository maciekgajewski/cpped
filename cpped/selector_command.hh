#pragma once

#include "commands.hh"

namespace cpped {

// Command that has another command as a parameter. Used to group commands
class selector_command : public base_command
{
public:

	selector_command(command_context& ctx, const std::string& prefix, const command_factory_list& subcommands);

	void on_text_changed(const std::string& text) override;
	bool on_enter_pressed() override;
	bool on_tab_pressed() override;

private:

	void display_hints(const std::string& filter);

	std::unique_ptr<base_command> next_command_;

	const command_factory_list& subcommands_;
};

class selector_command_factory : public icommand_factory
{
public:
	selector_command_factory(command_factory_list& subcommands)
		: subcommands_(subcommands) {}

	std::unique_ptr<base_command> create_command(command_context& ctx, const std::string& prefix) const override
	{
		return std::make_unique<selector_command>(ctx, prefix, subcommands_);
	}

private:
	const command_factory_list& subcommands_;
};

}
