#pragma once

#include "document_lib/document.hh"

#include "backend_lib/messages.hh"

#include <boost/filesystem.hpp>

#include <vector>
#include <string>
#include <map>

namespace cpped {

namespace backend { class endpoint; }

// Project - a collection of source files, targets, a code model
class project
{
public:
	project(backend::endpoint& ep);

	void open_cmake_project(const boost::filesystem::path& build_dir);
	document::document& open_file(const boost::filesystem::path& file);
	document::document& get_open_file(const boost::filesystem::path& file);

	template<typename OutIt>
	void get_all_project_files(OutIt out) const;
	template<typename OutIt>
	void get_all_open_files(OutIt out) const;

private:

	struct open_file_data
	{
		std::unique_ptr<document::document> document;
		std::uint64_t last_version_parsed = 0;
	};

	void on_document_changed(const document::document& doc);
	void on_file_tokens(const backend::messages::file_tokens_feed& token_feed);
	void request_parsing(const document::document& doc);

	backend::endpoint& endpoint_;

	// All project files
	std::vector<boost::filesystem::path> files_;

	// Currently open files
	std::map<boost::filesystem::path, open_file_data> open_files_;

	// asynchjronous parsing state
	bool parsing_in_progress_ = false;
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
