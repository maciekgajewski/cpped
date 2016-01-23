#pragma once

#include "editor_state.hh"

#include "document_lib/document.hh"

#include "backend_lib/messages.hh"

#include <boost/filesystem.hpp>

namespace cpped {

namespace ipc { class endpoint; }

// File open in the editor
class edited_file
{
public:

	edited_file(ipc::endpoint& endpoint, document::document&& doc, bool is_source, bool was_new);
	~edited_file();

	document::document& get_document() { return document_; }
	const document::document& get_document() const { return document_; }

	bool was_new() const { return was_new_; }

	void request_parsing(const boost::optional<document::document_position>& cursor_pos);

	std::vector<backend::messages::completion_record> get_completion(
		const document::document_position& cursor_pos);

	void save();

	boost::filesystem::path get_path() const { return document_.get_path(); }

	void on_file_tokens(const backend::messages::file_tokens_feed& token_feed);

	const boost::optional<editor_state>& get_editor_state() const { return editor_state_; }
	void set_editor_state(const editor_state& es) { editor_state_ = es; }

private:

	void send_parse_request(
		const boost::optional<document::document_position>& cursor_pos);

	ipc::endpoint& endpoint_;
	document::document document_;

	std::uint64_t last_version_parsed_ = 0;
	std::uint64_t last_version_send_ = 0;

	// If this is source file (ie parsing/completion available)
	bool is_source_;

	// If file was new at the time of creation
	bool was_new_ = false;

	// asynchronous parsing state
	bool parsing_in_progress_ = false;

	boost::optional<editor_state> editor_state_;
};

}
