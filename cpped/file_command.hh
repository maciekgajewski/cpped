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
	bool on_tab_pressed() override;

protected:

	boost::filesystem::path path_;

private:

	std::string tab_completion_;
};


class open_file_command : public file_command
{
public:
	open_file_command(command_context& ctx, const std::string& prefix)
		: file_command(ctx, prefix) {}

	bool on_enter_pressed() override;
};

using open_file_command_factory = generic_factory<open_file_command>;

class save_as_command : public file_command
{
public:
	save_as_command(command_context& ctx, const std::string& prefix)
		: file_command(ctx, prefix) {}

	bool on_enter_pressed() override;
};

using save_as_command_factory = generic_factory<save_as_command>;

}

