#pragma once

#include "ipc_lib/serialize.hh"

#include <boost/filesystem.hpp>

#include <cstdint>

// Messages exchanged between backend process (B) and it's workers (W)

namespace cpped { namespace backend { namespace worker_messages {

namespace fs = boost::filesystem;

// B->W
struct parse_file_request
{
	// messgaes here don't have to have different IDs than messgaes form messages.h
	// but I will use different values to ease debugging
	static const std::uint64_t ID = 1000;

	fs::path file;
	std::vector<std::string> commanline_params;
};
template<typename Writer> void serialize(Writer& writer, const parse_file_request& m)
{
	serialize(writer, m.file);
	serialize(writer, m.commanline_params);
}
template<typename Reader> void deserialize(Reader& reader, parse_file_request& m)
{
	deserialize(reader, m.file);
	deserialize(reader, m.commanline_params);
}

// W->B
struct parse_file_result
{
	static const std::uint64_t ID = 1001;
	fs::path file;
	std::vector<fs::path> includes;
};
template<typename Writer> void serialize(Writer& writer, const parse_file_result& m)
{
	serialize(writer, m.file);
	serialize(writer, m.includes);
}
template<typename Reader> void deserialize(Reader& reader, parse_file_result& m)
{
	deserialize(reader, m.file);
	deserialize(reader, m.includes);
}

}}}
