#include "compilation_unit.hh"

#include "log.hh"
#include "cpp_tokens.hh"

namespace cpped { namespace backend {

compilation_unit::compilation_unit(const boost::filesystem::path& path, clang::index& idx)
	: path_(path), index_(idx)
{
}

void compilation_unit::parse(const std::vector<CXUnsavedFile>& unsaved_data)
{
	assert(!is_parsed());

	LOG("Starting parsing file " << path_);

	std::vector<const char*> cmdline;
	cmdline.reserve(compilation_commands_.size()+1);
	assert(compilation_commands_.size() > 1);

	std::transform(
		compilation_commands_.begin(), compilation_commands_.end(),
		std::back_inserter(cmdline),
		[&](const std::string& c) { return c.c_str(); });

	translation_unit_.parse(
		index_,
		path_.string().c_str(),
		unsaved_data,
		cmdline);

	LOG("Finished parsing file " << path_);
}

void compilation_unit::reparse(const std::vector<CXUnsavedFile>& unsaved_data)
{
	// TODO
}

std::vector<document::token> compilation_unit::get_tokens_for_file(const boost::filesystem::path& path, const std::vector<char>& data) const
{
	return get_cpp_tokens(
		translation_unit_, path, data);
}

}}
