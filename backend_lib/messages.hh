#pragma once

#include "serialize.hh"

#include <boost/filesystem.hpp>

// Messages send between frontend (F) and backend (B) processes

namespace cpped { namespace backend { namespace messages {

namespace fs = boost::filesystem;

// F->B reuest to stop
struct stop
{
};
template<typename Writer> void serialize(Writer&, const stop&) {}
template<typename Reader> void deserialize(Reader&, stop&) {}

// F->B - rerquest to open cmake project, parse oll the files, build code model
struct open_cmake_project
{
	fs::path build_dir;
};

template<typename Writer> void serialize(Writer& writer, const open_cmake_project& m)
{
	serialize(writer, m.build_dir);
}
template<typename Reader> void deserialize(Reader& reader, open_cmake_project& m)
{
	deserialize(reader, m.build_dir);
}

// F->B. backend replies with open_file_data
struct open_file_request
{
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
	fs::path file;
	std::string error;
	std::string data; // TODO be smarter about large buffers, use copy-less structures
};

}}}
