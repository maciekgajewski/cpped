#include "file_command.hh"

#include <boost/range/algorithm.hpp>

namespace cpped {

namespace fs = boost::filesystem;

file_command::file_command(command_context& ctx, const std::string& prefix)
	: base_command(ctx, prefix)
{
}

void file_command::on_text_changed(const std::string& text)
{
	auto token_its = get_first_token(text);
	std::string token(token_its.first, token_its.second);	path_ = text;

	path_ = token;
	std::string filename;
	fs::path dir;
	if (path_.filename() == "/" || path_.filename() == ".")
	{
		dir = path_;
	}
	else
	{
		dir = path_.parent_path();
		if (dir.empty())
			dir = fs::current_path();
		filename = path_.filename().string();
	}

	try
	{
		std::vector<nct::list_widget::list_item> items;
		fs::directory_iterator it(dir);

		boost::optional<std::string> common_prefix;
		for(const fs::directory_entry& entry : it)
		{
			std::string name = entry.path().filename().string();
			if (name.find(filename) == 0)
			{
				if (entry.status().type() == fs::directory_file)
				{
					name += "/";
				}
				items.push_back(nct::list_widget::list_item{name});

				if (!common_prefix)
				{
					common_prefix = name;
				}
				else
				{
					auto pair = boost::mismatch(*common_prefix, name);
					common_prefix.emplace(name.begin(), pair.second);
				}
			}
		}

		if (common_prefix)
		{
			tab_completion_ = *common_prefix;
		}
		else
		{
			tab_completion_.clear();
		}

		boost::sort(items, [](const auto& it1, const auto& it2)
		{
			return it1.text < it2.text;
		});

		unsigned junk_len = (token_its.first - text.begin());
		ctx_.show_hints(prefix_.size() + junk_len, items);
	}
	catch(...)
	{
		// just ignore
		ctx_.show_hints(0, {});
		tab_completion_.clear();
	}
}

bool file_command::on_tab_pressed()
{
	if (!tab_completion_.empty())
	{
		fs::path new_path;
		if (path_.filename() == "/" || path_.filename() == ".")
		{
			new_path = path_ / tab_completion_;
		}
		else
		{
			new_path = path_.parent_path() / tab_completion_;
		}

		ctx_.set_text(prefix_ + new_path.string());
	}
	return false;
}

bool open_file_command::on_enter_pressed()
{
	if (fs::is_regular_file(path_))
	{
		ctx_.open_file(fs::canonical(path_));
		return true;
	}
	return false;
}



}
