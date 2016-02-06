#include "edited_file.hh"

#include "ipc_lib/endpoint.hh"

namespace cpped {

edited_file::edited_file(ipc::endpoint& endpoint, document::document&& doc, file_category category, bool was_new)
	: endpoint_(endpoint), document_(std::move(doc)), category_(category), was_new_(was_new)
{
	document_.document_changed_signal.connect(
		[this]() {
			if (last_have_unsaved_changes_ != document_.has_unsaved_changes())
			{
				status_changed_signal();
				last_have_unsaved_changes_ = document_.has_unsaved_changes();
			}
		});
}

edited_file::~edited_file()
{
	// TODO send 'closed' to the backend
}

void edited_file::send_parse_request(
	const boost::optional<document::document_position>& cursor_pos)
{
	backend::messages::document_changed_feed feed;
	feed.path = document_.get_path();
	feed.version = document_.get_current_version();
	feed.data.assign(document_.get_raw_data().begin(), document_.get_raw_data().end());
	feed.cursor_position = cursor_pos;

	endpoint_.send_message(feed);

	last_version_send_ = document_.get_current_version();
}


void edited_file::request_parsing(const boost::optional<document::document_position>& cursor_pos)
{
	if (category_ != file_category::source)
	{
		return; // ignore for non-source files
	}

	if (!parsing_in_progress_)
	{
		send_parse_request(cursor_pos);
		parsing_in_progress_ = true;
	}
}

std::vector<backend::messages::completion_record> edited_file::get_completion(const document::document_position& cursor_pos)
{
	backend::messages::complete_at_reply reply;

	if (last_version_send_ < document_.get_current_version())
	{
		backend::messages::document_changed_feed request;
		request.path = document_.get_path();
		request.version = document_.get_current_version();
		request.data.assign(document_.get_raw_data().begin(), document_.get_raw_data().end());
		request.cursor_position = cursor_pos;

		endpoint_.send_sync_request(request, reply);
		last_version_send_ = document_.get_current_version();
		parsing_in_progress_ = true;
	}
	else
	{
		backend::messages::complete_at_request request;
		request.path = document_.get_path();
		request.cursor_position = cursor_pos;

		endpoint_.send_sync_request(request, reply);

	}

	return reply.results;
}

void edited_file::ensure_latest_data_send()
{
	if (last_version_send_ < document_.get_current_version())
	{
		backend::messages::document_changed_feed change_feed;
		change_feed.path = document_.get_path();
		change_feed.version = document_.get_current_version();
		change_feed.data.assign(document_.get_raw_data().begin(), document_.get_raw_data().end());

		endpoint_.send_message(change_feed);
		last_version_send_ = document_.get_current_version();
		parsing_in_progress_ = true;
	}
}

void edited_file::save()
{
	if (document_.has_unsaved_changes())
	{
		ensure_latest_data_send();

		backend::messages::save_reply reply;
		backend::messages::save_request request;
		request.path = document_.get_path();

		endpoint_.send_sync_request(request, reply);

		if (!reply.error.empty())
		{
			throw std::runtime_error(reply.error);
		}

		document_.set_saved(reply.version);
		last_have_unsaved_changes_ = false;
		status_changed_signal();
	}
}

void edited_file::save_as(const boost::filesystem::path& path)
{
	backend::messages::save_as_request request;
	request.old_path = get_path();
	request.new_path = path;
	request.version = document_.get_current_version();
	request.data.assign(document_.get_raw_data().begin(), document_.get_raw_data().end());

	backend::messages::save_reply reply;
	endpoint_.send_sync_request(request, reply);

	if (!reply.error.empty())
	{
		throw std::runtime_error(reply.error);
	}

	document_.set_path(path);
	document_.set_saved(reply.version);
	last_have_unsaved_changes_ = false;
	status_changed_signal();
}

void edited_file::on_file_tokens(const backend::messages::file_tokens_feed& token_feed)
{
	document_.set_tokens(token_feed.version, token_feed.tokens);
	last_version_parsed_ = token_feed.version;
	parsing_in_progress_ = false;

	// if document changed in the meantime, re-send parsing
	while(last_version_send_ < document_.get_current_version())
	{
		send_parse_request(boost::none);
	}
}

std::string edited_file::get_name() const
{
	if (document_.get_path().empty())
	{
		return unsaved_name_;
	}
	else
	{
		return document_.get_path().string();
	}
}

}
