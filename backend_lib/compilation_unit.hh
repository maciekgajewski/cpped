#pragma once

#include "cpp_tokens.hh"

#include "clang_lib/clang.hh"

#include "document_lib/document_data.hh"

#include <boost/container/flat_set.hpp>
#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

namespace cpped { namespace backend {

// Translation unit
class compilation_unit
{
public:

	boost::signals2::signal<void()> includes_changed_signal;

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
	token_data get_tokens_with_diagnostics(const boost::filesystem::path& path, const std::vector<char>& data) const;
	boost::filesystem::path get_path() const { return path_; }

	bool includes(const boost::filesystem::path& file) const
	{
		return included_files_.find(file) != included_files_.end();
	}

	void mark_dirty();

private:

	void update_includes();

	boost::filesystem::path path_;
	boost::container::flat_set<boost::filesystem::path> included_files_;
	clang::index& index_;
	clang::translation_unit translation_unit_;
	std::vector<std::string> compilation_flags_;
	bool needs_parsing_ = true;
	bool needs_reparsing_ = false;
};

}}
