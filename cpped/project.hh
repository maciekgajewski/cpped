#pragma once

#include "edited_file.hh"

#include "document_lib/document.hh"

#include "backend_lib/messages.hh"

#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

#include <vector>
#include <string>
#include <deque>

namespace cpped {

namespace ipc { class endpoint; }

// Project - a collection of source files, targets, a code model
class project
{
public:

	boost::signals2::signal<void(const std::string&)> status_signal;

	project(ipc::endpoint& ep);

	void open_cmake_project(const boost::filesystem::path& build_dir);
	edited_file& open_file(const boost::filesystem::path& path);
	edited_file& get_open_file(const boost::filesystem::path& path);
	std::unique_ptr<edited_file> make_unsaved_file();



	// Returns (via output iterator) all open files (paths)
	template<typename OutIt>
	void get_all_project_files(OutIt out) const;
	template<typename OutIt>
	void get_all_open_files(OutIt out) const;

private:

	void on_file_tokens(const backend::messages::file_tokens_feed& token_feed);

	void send_parse_request(
		const document::document& doc,
		const boost::optional<document::document_position>& cursor_pos);

	edited_file* find_file(const boost::filesystem::path& path);

	ipc::endpoint& endpoint_;

	// All project files
	std::vector<boost::filesystem::path> files_;

	// Currently open files

	std::deque<std::unique_ptr<edited_file>> open_files_;

};

project load_cmake_project(const std::string& build_directory);


template<typename OutIt>
void project::get_all_project_files(OutIt out) const
{
	std::copy(files_.begin(), files_.end(), out);
}

template<typename OutIt>
void project::get_all_open_files(OutIt out) const
{
	std::transform(open_files_.begin(), open_files_.end(), out,
		[](const auto& ef_ptr) { return ef_ptr->get_path(); });
}

}
