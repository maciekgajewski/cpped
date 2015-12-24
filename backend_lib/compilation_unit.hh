#pragma once

#include "clang_lib/clang.hh"

#include "document_lib/document_data.hh"

#include <boost/filesystem.hpp>

namespace cpped { namespace backend {

// Translation unit
class compilation_unit
{
public:

	compilation_unit(const boost::filesystem::path& path, clang::index& idx);

	bool is_parsed() const { return !translation_unit_.is_null(); }
	void parse(const std::vector<CXUnsavedFile>& unsaved_data);
	void reparse(const std::vector<CXUnsavedFile>& unsaved_data);

	template<typename Container>
	void set_compilation_commands(const Container& in)
	{
		compilation_commands_.assign(std::begin(in), std::end(in));
	}

	std::vector<document::token> get_tokens_for_file(const boost::filesystem::path& path, const std::vector<char>& data) const;

private:

	boost::filesystem::path path_;
	clang::index& index_;
	clang::translation_unit translation_unit_;
	std::vector<std::string> compilation_commands_;
};

}}
