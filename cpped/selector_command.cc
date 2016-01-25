#include "selector_command.hh"

#include <boost/range/algorithm.hpp>

namespace cpped {

selector_command::selector_command(const command_context& ctx, const std::string& prefix, const command_factory_list& subcommands)
	: ctx_(ctx)
	, prefix_(prefix)
	, subcommands_(subcommands)
{
}

void selector_command::on_text_changed(const std::string& text)
{
	if (next_command_)
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
		[&](const auto& p) { return p.name == token; });
	if (it == subcommands_.end())
	{
		display_hints(token);
	}
	else
	{
		// create subcommand
		std::string prefix = prefix_ + std::string(text.begin(), text.begin() + token_end);
		next_command_ = it->factory->create_command(ctx_, prefix);
	}
}

bool selector_command::on_enter_pressed()
{
	if (next_command_)
		return next_command_->on_enter_pressed();
	else
	{
		if (ctx_.hint_list->is_visible())
		{
			auto item = ctx_.hint_list->get_current_item();
			if (item)
			{
				ctx_.command_editor->set_text(prefix_+ item->text + " ");
			}
		}
		return false;
	}
}

void selector_command::display_hints(const std::string& filter)
{
	// items
	std::vector<nct::list_widget::list_item> items;
	items.reserve(subcommands_.size());
	for(const auto& subcommand : subcommands_)
	{
		if (filter.empty() || subcommand.name.find(filter) != std::string::npos)
		{
			items.push_back(
				{subcommand.name, subcommand.help});
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
		pos.x = ctx_.editor_pos.x + prefix_.length();
		pos.y = ctx_.editor_pos.y - sz.h;
		ctx_.hint_list->move(pos, sz);
		ctx_.hint_list->show();
	}
}

}
