#pragma once

#include "document_lib/document.hh"

#include "backend_lib/messages.hh"

#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

#include <vector>
#include <string>
#include <map>

namespace cpped {

namespace ipc { class endpoint; }

// Project - a collection of source files, targets, a code model
class project
{
public:

	struct open_file_result
	{
		document::document&	document;
		bool was_new;
	};

	boost::signals2::signal<void(const std::string&)> status_signal;

	project(ipc::endpoint& ep);

	void open_cmake_project(const boost::filesystem::path& build_dir);
	open_file_result open_file(const boost::filesystem::path& file);
	document::document& get_open_file(const boost::filesystem::path& file);

	template<typename OutIt>
	void get_all_project_files(OutIt out) const;
	template<typename OutIt>
	void get_all_open_files(OutIt out) const;

	// TODO create project_file class, make these two methods
	void request_parsing(
		const document::document& doc,
		const boost::optional<document::document_position>& cursor_pos);

	std::vector<backend::messages::completion_record> get_completion(
		const boost::filesystem::path& file,
		const document::document_position& cursor_pos);

	void save_file(const boost::filesystem::path& file);

private:

	struct open_file_data
	{
		std::unique_ptr<document::document> document;
		std::uint64_t last_version_parsed;
		std::uint64_t last_version_send;
		bool is_source;
	};

	void on_file_tokens(const backend::messages::file_tokens_feed& token_feed);

	void send_parse_request(
		const document::document& doc,
		const boost::optional<document::document_position>& cursor_pos);

	ipc::endpoint& endpoint_;

	// All project files
	std::vector<boost::filesystem::path> files_;

	// Currently open files
	std::map<boost::filesystem::path, open_file_data> open_files_;

	// asynchjronous parsing state
	bool parsing_in_progress_ = false;

	// outstanding parsing requests
	std::map<boost::filesystem::path, boost::optional<document::document_position>> outstanding_parsing_requests_;
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
		[](const auto& p) { return p.first; });
}

}
