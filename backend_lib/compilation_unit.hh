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

	bool needs_parsing() const { return needs_parsing_; }
	void parse(const std::vector<CXUnsavedFile>& unsaved_data);
	void reparse(const std::vector<CXUnsavedFile>& unsaved_data);

	template<typename Container>
	void set_compilation_flags(const Container& in)
	{
		std::vector<std::string> flags(std::begin(in), std::end(in));

		if (flags != compilation_flags_)
		{
			needs_parsing_ = true;
			compilation_flags_ = std::move(flags);
		}
	}

	std::vector<std::string> get_compilation_flags() const { return compilation_flags_; }

	std::vector<document::token> get_tokens_for_file(const boost::filesystem::path& path, const std::vector<char>& data) const;

private:

	boost::filesystem::path path_;
	clang::index& index_;
	clang::translation_unit translation_unit_;
	std::vector<std::string> compilation_flags_;
	bool needs_parsing_ = true;
};

}}
