#include "commands.hh"

#include <boost/range/algorithm.hpp>

namespace cpped {

std::pair<std::string, std::unique_ptr<icommand_factory>> make_command(
	const char* text, std::unique_ptr<icommand_factory>&& ptr)
	{
		return std::pair<std::string, std::unique_ptr<icommand_factory>>{text, std::move(ptr)};
	}

static command_factory_list root_commands
{
	{"goto", nullptr},
	{"find", nullptr},
};


selector_command::selector_command(const command_context& ctx, unsigned shift, const command_factory_list& subcommands)
	: ctx_(ctx)
	, shift_(shift)
	, subcommands_(subcommands)
{
}

void selector_command::on_text_changed(const std::string& text)
{
	if (!prefix_.empty())
	{
		// TODO verify prefix
	}

	// parse
	auto token_begin = text.find_first_not_of(' ');
	auto token_end = text.find_first_of(' ', token_begin);

	std::string token(
		token_begin == std::string::npos ?
			text.begin() : text.begin() + token_begin,
		token_end == std::string::npos ?
			text.end() : text.begin() + token_end);

	// do we know this guy?
	auto it = boost::find_if(subcommands_,
		[&](const auto& p) { return p.first == token; });
	if (it == subcommands_.end())
	{
		display_hints(token);
	}
	else
	{
		// create subcommand
		prefix_.assign(text.begin(), text.begin() + token_end);
		assert(false);
		// TODO
	}
}

bool selector_command::on_enter_pressed()
{
	if (next_command_)
		return next_command_->on_enter_pressed();
	else
		return false;
}

void selector_command::display_hints(const std::string& filter)
{
	// items
	std::vector<nct::list_widget::list_item> items;
	items.reserve(subcommands_.size());
	for(const auto& subcommand : subcommands_)
	{
		if (filter.empty() || subcommand.first.find(filter) != std::string::npos)
		{
			items.push_back(
				{subcommand.first});
		}
	}

	if (items.empty())
	{
		ctx_.hint_list->hide();
	}
	else
	{
		ctx_.hint_list->set_items(items);

		nct::size sz = ctx_.hint_list->get_content_size();
		nct::position pos;
		pos.x = ctx_.editor_pos.x + shift_;
		pos.y = ctx_.editor_pos.y - sz.h;
		ctx_.hint_list->move(pos, sz);
		ctx_.hint_list->show();
	}
}

std::unique_ptr<icommand> make_root_command(const command_context& ctx)
{
	return std::make_unique<selector_command>(ctx, 0, root_commands);
}

}
