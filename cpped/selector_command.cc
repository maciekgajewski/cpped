#include "selector_command.hh"

#include <boost/range/algorithm.hpp>

namespace cpped {

selector_command::selector_command(const command_context& ctx, const std::string& prefix, const command_factory_list& subcommands)
	: base_command(ctx, prefix)
	, subcommands_(subcommands)
{
}

static bool starts_with(const std::string& text, const std::string& prefix)
{
	return
		prefix.length() <= text.length()
		&& std::equal(prefix.begin(), prefix.end(), text.begin(), text.begin() + prefix.length());
}

void selector_command::on_text_changed(const std::string& text)
{
	if (next_command_)
	{
		if (starts_with(text, next_command_->get_prefix()))
		{
			std::string reminder = text.substr(next_command_->get_prefix().length());
			next_command_->on_text_changed(reminder);
			return;
		}
		else
		{
			next_command_.reset();
		}
	}

	auto token_its = get_first_token(text);
	std::string token(token_its.first, token_its.second);

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
		std::string prefix = prefix_ + std::string(text.begin(), token_its.second);
		next_command_ = it->factory->create_command(ctx_, prefix);
		if (token_its.second != text.end())
		{
			next_command_->on_text_changed(std::string(token_its.second, text.end()));
		}
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
	// build item list
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
	show_hints(items);
}

}
