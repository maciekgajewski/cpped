#pragma once

#include "commands.hh"

#include <boost/filesystem.hpp>

namespace cpped {

// command that cosumes path
class file_command : public base_command
{
public:

	file_command(command_context& ctx, const std::string& prefix);

	void on_text_changed(const std::string& text) override;
	bool on_enter_pressed() override;
	bool on_tab_pressed() override;

private:

	boost::filesystem::path path_;
	std::string tab_completion_;
};

struct file_command_factory final : public icommand_factory
{
	std::unique_ptr<base_command> create_command(command_context& ctx, const std::string& prefix) const override
	{
		return std::make_unique<file_command>(ctx, prefix);
	}
};

}

