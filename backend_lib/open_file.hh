#pragma once

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

private:

	void load_from_disk();

	boost::filesystem::path path_;
	std::vector<char> data_;

	compilation_unit* unit_ = nullptr;

};

}}
