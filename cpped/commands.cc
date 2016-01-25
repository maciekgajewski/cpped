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


std::unique_ptr<base_command> make_root_command(const command_context& ctx)
{
	return std::make_unique<selector_command>(ctx, std::string(), root_commands);
}

void base_command::show_hints(const std::vector<nct::list_widget::list_item>& items)
{
	if (items.empty())
	{
		ctx_.hint_list->hide();
	}
	else
	{
		ctx_.hint_list->set_items(items);

		nct::size content_size = ctx_.hint_list->get_content_size();
		nct::size sz;
		sz.h = std::min<int>(content_size.h, ctx_.command_editor->get_global_position().y);
		sz.w = std::min<int>(content_size.w, ctx_.editor_width - prefix_.length());

		nct::position pos;
		pos.x = ctx_.editor_pos.x + prefix_.length();
		pos.y = ctx_.editor_pos.y - sz.h;
		ctx_.hint_list->move(pos, sz);
		ctx_.hint_list->show();
	}
}

}
