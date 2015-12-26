#pragma once

#include "document_lib/document_data.hh"

#include "clang_lib/clang.hh"

#include <boost/filesystem.hpp>

#include <memory>

namespace cpped { namespace backend {

class compilation_unit;

// Holds data of an open file, responsible for loading, saving and re-parsing
class open_file
{
public:

	open_file(const boost::filesystem::path& path);

	void set_compilation_unit(compilation_unit* cu) { unit_ = cu; provisional_unit_.reset(); }

	// Provisional unit is a temprary unit for header files, used unles a better one is found
	void set_provisional_compilation_unit(std::unique_ptr<compilation_unit> u)
	{
		provisional_unit_ = std::move(u);
		unit_ = provisional_unit_.get();
	}

	const std::vector<char>& get_data() const { return data_; }

	void set_data(const std::vector<char>& d)
	{
		data_ = d;
		has_unsaved_data_ = true;
	}

	void set_data(const std::string& d)
	{
		data_.assign(d.begin(), d.end());
		has_unsaved_data_ = true;
	}

	std::vector<document::token> parse(const std::vector<CXUnsavedFile>& unsaved_data);

	bool has_unsaved_data() const { return has_unsaved_data_; }

	const boost::filesystem::path& get_path() const{ return path_; }

private:

	void load_from_disk();

	boost::filesystem::path path_;
	std::vector<char> data_;

	compilation_unit* unit_ = nullptr;
	std::unique_ptr<compilation_unit> provisional_unit_;
	bool has_unsaved_data_ = false;

};

}}
