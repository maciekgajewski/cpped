#include "compilation_unit.hh"

#include "log.hh"
#include "completion.hh"

namespace cpped { namespace backend {

namespace fs = boost::filesystem;

compilation_unit::compilation_unit(const boost::filesystem::path& path, clang::index& idx)
	: path_(path), index_(idx)
{
}

void compilation_unit::parse(const std::vector<CXUnsavedFile>& unsaved_data, parse_mode mode)
{
	assert(needs_parsing());

	LOG("Starting parsing file " << path_);

	std::vector<const char*> cmdline;
	cmdline.reserve(compilation_flags_.size()+1);
	assert(compilation_flags_.size() > 1);

	std::transform(
		compilation_flags_.begin(), compilation_flags_.end(),
		std::back_inserter(cmdline),
		[&](const std::string& c) { return c.c_str(); });

	unsigned parsingOptions = 0;
	if (mode == parse_mode::full)
	{
		parsingOptions = CXTranslationUnit_CacheCompletionResults
			 | CXTranslationUnit_PrecompiledPreamble
			 | CXTranslationUnit_IncludeBriefCommentsInCodeCompletion
			 | CXTranslationUnit_DetailedPreprocessingRecord;
	}
	else
	{
		parsingOptions = CXTranslationUnit_SkipFunctionBodies|CXTranslationUnit_PrecompiledPreamble;
	}

	translation_unit_.parse(
		index_,
		path_.string().c_str(),
		unsaved_data,
		cmdline,
		parsingOptions);

	needs_parsing_ = false;
	if (mode == parse_mode::full)
	{
		needs_reparsing_ = false;
	}
	else
	{
		needs_reparsing_ = true;
	}

	update_includes();

	LOG("Finished parsing file " << path_);
}

void compilation_unit::reparse(const std::vector<CXUnsavedFile>& unsaved_data)
{
	if (needs_reparsing_)
	{
		LOG("Starting reparsing file " << path_);
		translation_unit_.reparse(unsaved_data);
		update_includes();
		LOG("Finished reparsing file " << path_);
		needs_reparsing_ = false;
	}
	else
	{
		LOG("No need to reparse " << path_);
	}
}

std::vector<document::token> compilation_unit::get_tokens_for_file(const boost::filesystem::path& path, const std::vector<char>& data) const
{
	return get_cpp_tokens(
		translation_unit_, path, data);
}

document::token_data compilation_unit::get_tokens_with_diagnostics(const boost::filesystem::path& path, const std::vector<char>& data) const
{
	return get_cpp_tokens_with_diagnostics(
				translation_unit_, path, data);
}

std::vector<messages::completion_record> compilation_unit::complete_at(
	const std::vector<CXUnsavedFile>& unsaved_data,
	const boost::filesystem::path& path,
	const document::document_position& pos)
{
	assert(!translation_unit_.is_null());

	clang::code_completion_results results = translation_unit_.code_complete_at(
		path.c_str(), pos.line+1, pos.column+1, unsaved_data);
	needs_reparsing_ = false;
	return process_completion_results(results);
}

void compilation_unit::mark_dirty()
{
	needs_reparsing_ = true;
	LOG("Marked as dirty: " << path_);
}

void compilation_unit::update_includes()
{
	assert(!translation_unit_.is_null());
	LOG("looking for includes");

	boost::container::flat_set<fs::path> includes;
	includes.reserve(std::max<std::size_t>(32, included_files_.size()));

	clang::cursor cursor = translation_unit_.get_cursor();
	cursor.visit_children(
		[&](const clang::cursor& visited_cursor, const clang::cursor& /*parent*/)
		{
			if (visited_cursor.get_kind() == CXCursor_InclusionDirective)
			{
				clang::source_file file = visited_cursor.get_included_file();
				if (!file.is_null())
				{
					fs::path p(file.get_name().c_str());
					includes.insert(fs::canonical(p));
				}
			}
			return CXChildVisit_Continue;
		});

	LOG("finished looking for includes, includes found=" << includes.size());

	if (includes != included_files_)
	{
		included_files_	.swap(includes);
		includes_changed_signal();
	}
}

}}
