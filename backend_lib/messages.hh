#pragma once

#include "serialize.hh"

#include "document_lib/document_data.hh"

#include <boost/filesystem.hpp>

// Messages send between frontend (F) and backend (B) processes

namespace cpped { namespace backend { namespace messages {

namespace fs = boost::filesystem;

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
};
template<typename Writer> void serialize(Writer& writer, const open_file_reply& m)
{
	serialize(writer, m.file);
	serialize(writer, m.error);
	serialize(writer, m.data);
}
template<typename Reader> void deserialize(Reader& reader, open_file_reply& m)
{
	deserialize(reader, m.file);
	deserialize(reader, m.error);
	deserialize(reader, m.data);
}

// B->F
struct file_tokens_feed
{
	static const std::uint64_t ID = 5;
	fs::path file;
	std::uint64_t version;
	std::vector<document::token> tokens;
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

}}}
