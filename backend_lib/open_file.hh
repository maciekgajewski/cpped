#pragma once

#include "compilation_unit.hh"
#include "messages.hh"

#include "document_lib/document_data.hh"

#include "clang_lib/clang.hh"

#include <boost/filesystem.hpp>

#include <memory>

namespace cpped { namespace backend {

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
	bool uses_provisional_unit() const { return !!provisional_unit_; }

	const std::vector<char>& get_data() const { return data_; }

	void set_data(const std::vector<char>& d, std::uint64_t version)
	{
		data_ = d;
		version_ = version;
		has_unsaved_data_ = true;
	}

	void set_data(const std::string& d, std::uint64_t version)
	{
		data_.assign(d.begin(), d.end());
		version_ = version;
		has_unsaved_data_ = true;
	}

	document::token_data parse(const std::vector<CXUnsavedFile>& unsaved_data);
	std::vector<messages::completion_record> complete_at(
		const std::vector<CXUnsavedFile>& unsaved_data,
		const document::document_position& pos);
	bool is_source() const { return unit_ != nullptr; }

	bool has_unsaved_data() const { return has_unsaved_data_; }

	const boost::filesystem::path& get_path() const{ return path_; }
	std::uint64_t get_version() const { return version_; }
	bool was_new() const { return was_new_; }

private:

	void load_from_disk();

	boost::filesystem::path path_;
	std::vector<char> data_;

	compilation_unit* unit_ = nullptr;
	std::unique_ptr<compilation_unit> provisional_unit_;
	bool has_unsaved_data_ = false;
	std::uint64_t version_ = 0;
	bool was_new_ = false; // if file was newly created
};

}}
