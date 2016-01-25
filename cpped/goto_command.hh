#pragma once

#include "commands.hh"

#include <boost/filesystem.hpp>

#include <set>

namespace cpped {

// 'goto' command implements roughly the functionality of QtCreator's ctrl-k locator widget
class goto_command : public base_command
{
public:

	goto_command(const command_context& ctx, const std::string& prefix);

	void on_text_changed(const std::string& text) override;
	bool on_enter_pressed() override;

private:

	std::set<boost::filesystem::path> files_;
};

struct goto_command_factory final : public icommand_factory
{
	std::unique_ptr<base_command> create_command(const command_context& ctx, const std::string& prefix) const override
	{
		return std::make_unique<goto_command>(ctx, prefix);
	}
};

}

