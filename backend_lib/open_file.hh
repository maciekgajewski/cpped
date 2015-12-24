#pragma once

#include "document_lib/document_data.hh"

#include "clang_lib/clang.hh"

#include <boost/filesystem.hpp>

namespace cpped { namespace backend {

class compilation_unit;

// Holds data of an open file, responsible for loading, saving and re-parsing
class open_file
{
public:

	open_file(const boost::filesystem::path& path);

	void set_compilation_unit(compilation_unit* cu) { unit_ = cu; }

	const std::vector<char>& get_data() const { return data_; }

	std::vector<document::token> parse(const std::vector<CXUnsavedFile>& unsaved_data);

	bool has_unsaved_data() const { return false; }

private:

	void load_from_disk();

	boost::filesystem::path path_;
	std::vector<char> data_;

	compilation_unit* unit_ = nullptr;

};

}}
