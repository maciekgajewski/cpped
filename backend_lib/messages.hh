#pragma once

#include "ipc_lib/serialize.hh"

#include "cpp_tokens.hh"

#include "document_lib/document_data.hh"

#include <boost/filesystem.hpp>

// Messages send between frontend (F) and backend (B) processes

namespace cpped { namespace backend { namespace messages {

namespace fs = boost::filesystem;

//single completion entry. Used in few messages
struct completion_record
{
	std::string text;
	std::string hint;
};
template<typename Writer> void serialize(Writer& writer, const completion_record& m)
{
	serialize(writer, m.text);
	serialize(writer, m.hint);
}
template<typename Reader> void deserialize(Reader& reader, completion_record& m)
{
	deserialize(reader, m.text);
	deserialize(reader, m.hint);
}

// F->B reuest to stop
struct stop
{
	static const std::uint64_t ID = 0;
};
template<typename Writer> void serialize(Writer&, const stop&) {}
template<typename Reader> void deserialize(Reader&, stop&) {}

// F->B - rerquest to open cmake project, parse oll the files, build code model
struct open_cmake_project_request
{
	static const std::uint64_t ID = 1;
	fs::path build_dir;
};

template<typename Writer> void serialize(Writer& writer, const open_cmake_project_request& m)
{
	serialize(writer, m.build_dir);
}
template<typename Reader> void deserialize(Reader& reader, open_cmake_project_request& m)
{
	deserialize(reader, m.build_dir);
}

// B->F, feed with all project files
struct open_cmake_project_reply
{
	static const std::uint64_t ID = 2;
	std::string error;
	std::vector<fs::path> files;
};

template<typename Writer> void serialize(Writer& writer, const open_cmake_project_reply& m)
{
	serialize(writer, m.error);
	serialize(writer, m.files);
}
template<typename Reader> void deserialize(Reader& reader, open_cmake_project_reply& m)
{
	deserialize(reader, m.error);
	deserialize(reader, m.files);
}

// F->B. backend replies with open_file_data
struct open_file_request
{
	static const std::uint64_t ID = 3;
	fs::path file;
};
template<typename Writer> void serialize(Writer& writer, const open_file_request& m)
{
	serialize(writer, m.file);
}
template<typename Reader> void deserialize(Reader& reader, open_file_request& m)
{
	deserialize(reader, m.file);
}

// B->F
struct open_file_reply
{
	static const std::uint64_t ID = 4;
	fs::path file;
	std::string error;
	std::string data; // TODO be smarter about large buffers, use copy-less structures
	document::token_data tokens;
	bool new_file;
	bool parsed;
};
template<typename Writer> void serialize(Writer& writer, const open_file_reply& m)
{
	serialize(writer, m.file);
	serialize(writer, m.error);
	serialize(writer, m.data);
	serialize(writer, m.tokens);
	serialize(writer, m.new_file);
	serialize(writer, m.parsed);
}
template<typename Reader> void deserialize(Reader& reader, open_file_reply& m)
{
	deserialize(reader, m.file);
	deserialize(reader, m.error);
	deserialize(reader, m.data);
	deserialize(reader, m.tokens);
	deserialize(reader, m.new_file);
	deserialize(reader, m.parsed);
}

// B->F
struct file_tokens_feed
{
	static const std::uint64_t ID = 5;
	fs::path file;
	std::uint64_t version;
	document::token_data tokens;
};
template<typename Writer> void serialize(Writer& writer, const file_tokens_feed& m)
{
	serialize(writer, m.file);
	serialize(writer, m.version);
	serialize(writer, m.tokens);
}
template<typename Reader> void deserialize(Reader& reader, file_tokens_feed& m)
{
	deserialize(reader, m.file);
	deserialize(reader, m.version);
	deserialize(reader, m.tokens);
}

// F->B
struct document_changed_feed
{
	static const std::uint64_t ID = 6;
	fs::path file;
	std::uint64_t version;
	std::string data;
	boost::optional<document::document_position> cursor_position;
};
template<typename Writer> void serialize(Writer& writer, const document_changed_feed& m)
{
	serialize(writer, m.file);
	serialize(writer, m.version);
	serialize(writer, m.data);
	serialize(writer, m.cursor_position);
}
template<typename Reader> void deserialize(Reader& reader, document_changed_feed& m)
{
	deserialize(reader, m.file);
	deserialize(reader, m.version);
	deserialize(reader, m.data);
	deserialize(reader, m.cursor_position);
}

// F->B
struct complete_at_request
{
	static const std::uint64_t ID = 7;
	fs::path file;
	document::document_position cursor_position;
};
template<typename Writer> void serialize(Writer& writer, const complete_at_request& m)
{
	serialize(writer, m.file);
	serialize(writer, m.cursor_position);
}
template<typename Reader> void deserialize(Reader& reader, complete_at_request& m)
{
	deserialize(reader, m.file);
	deserialize(reader, m.cursor_position);
}

// B->F
struct complete_at_reply
{
	static const std::uint64_t ID = 8;
	std::vector<completion_record> results;
};
template<typename Writer> void serialize(Writer& writer, const complete_at_reply& m)
{
	serialize(writer, m.results);
}
template<typename Reader> void deserialize(Reader& reader, complete_at_reply& m)
{
	deserialize(reader, m.results);
}

// B->F
struct status_feed
{
	static const std::uint64_t ID = 9;
	std::string message;
};
template<typename Writer> void serialize(Writer& writer, const status_feed& m)
{
	serialize(writer, m.message);
}
template<typename Reader> void deserialize(Reader& reader, status_feed& m)
{
	deserialize(reader, m.message);
}

// F->B
struct save_request
{
	static const std::uint64_t ID = 10;
	fs::path file;
};
template<typename Writer> void serialize(Writer& writer, const save_request& m)
{
	serialize(writer, m.file);
}
template<typename Reader> void deserialize(Reader& reader, save_request& m)
{
	deserialize(reader, m.file);
}

// F->B
struct save_reply
{
	static const std::uint64_t ID = 11;
	fs::path file;
	std::uint64_t version;
	std::string error;
};
template<typename Writer> void serialize(Writer& writer, const save_reply& m)
{
	serialize(writer, m.file);
	serialize(writer, m.version);
	serialize(writer, m.error);
}
template<typename Reader> void deserialize(Reader& reader, save_reply& m)
{
	deserialize(reader, m.file);
	deserialize(reader, m.version);
	deserialize(reader, m.error);
}

}}}
