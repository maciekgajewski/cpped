#include "commands.hh"

#include "selector_command.hh"
#include "goto_command.hh"

namespace cpped {

std::pair<std::string, std::unique_ptr<icommand_factory>> make_command(
	const char* text, std::unique_ptr<icommand_factory>&& ptr)
	{
		return std::pair<std::string, std::unique_ptr<icommand_factory>>{text, std::move(ptr)};
	}

static command_factory_list root_commands
{
	{"goto", "        - go to file, symbol, line etc", new goto_command_factory},
	{"find", "<text>  - find text in file", nullptr},
};


std::unique_ptr<base_command> make_root_command(command_context& ctx)
{
	return std::make_unique<selector_command>(ctx, std::string(), root_commands);
}


}
