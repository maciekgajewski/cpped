#pragma once

#include "command_context.hh"

#include <nct/list_widget.hh>
#include <nct/line_edit.hh>

#include <string>
#include <memory>
#include <vector>

namespace cpped {

class main_window;

// command and command factory interfaces
class base_command
{
public:
	base_command(command_context& ctx, const std::string& prefix)
		: ctx_(ctx), prefix_(prefix)
	{}

	virtual void on_text_changed(const std::string& text) = 0;
	virtual bool on_enter_pressed() = 0;

	const std::string& get_prefix() const { return prefix_; }

protected:


	static std::pair<std::string::const_iterator, std::string::const_iterator> get_first_token(const std::string& text)
	{
		auto token_begin_it = std::find_if(
			text.begin(), text.end(), [](char c) { return c != ' '; });
		auto token_end_it = std::find_if(
			token_begin_it, text.end(), [](char c) { return c == ' '; });
		return std::make_pair(token_begin_it, token_end_it);
	}


	command_context& ctx_;
	std::string prefix_;
};

class icommand_factory
{
public:
	virtual std::unique_ptr<base_command> create_command(command_context& ctx, const std::string& prefix) const = 0;
};

struct command_entry
{
	std::string name;
	std::string help;
	icommand_factory* factory;
};

using command_factory_list = std::vector<command_entry>;

// root command
std::unique_ptr<base_command> make_root_command(command_context& ctx);

}
