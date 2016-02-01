#include "goto_command.hh"

#include "main_window.hh"

namespace cpped {

namespace fs = boost::filesystem;

goto_command::goto_command(command_context& ctx, const std::string& prefix)
	: base_command(ctx, prefix)
{
	const project& pr = ctx_.get_project();

	pr.get_all_project_files(std::inserter(files_, files_.end()));

	std::vector<edited_file*> open_files;
	pr.get_all_open_files(std::back_inserter(open_files));

	for(edited_file* ef : open_files)
	{
		// skip unsaved files
		const fs::path p = ef->get_path();
		if (!p.empty())
		{
			files_.insert(p);
		}
	}
}

void goto_command::on_text_changed(const std::string& text)
{
	// parse
	auto token_its = get_first_token(text);
	std::string token(token_its.first, token_its.second);

	std::vector<nct::list_widget::list_item> items;
	items.reserve(files_.size());
	for(const fs::path& path : files_)
	{
		std::string fn = path.filename().string();
		if (fn.find(token) != std::string::npos)
		{
			items.push_back(
				nct::list_widget::list_item{fn, path.string(), path});
		}
	}

	ctx_.show_hints(prefix_.length(), items);
}

bool goto_command::on_enter_pressed()
{
	auto item = ctx_.get_current_item();
	if (item)
	{
		fs::path path = boost::any_cast<fs::path>(item->data);
		ctx_.open_file(path);
	}
	return true;
}

}
