#include "clang.hh"

#include <stdexcept>
#include <vector>

namespace cpped { namespace clang {

translation_unit::translation_unit(index& idx, const char* filename, const char* unsaved_data, std::size_t unsaved_data_size)
{
	CXUnsavedFile* uf_ptr = nullptr;
	CXUnsavedFile unsaved_file;
	if (unsaved_data)
	{
		unsaved_file.Contents = unsaved_data;
		unsaved_file.Length = unsaved_data_size;
		unsaved_file.Filename = filename;
		uf_ptr = &unsaved_file;
	}

	std::vector<const char*> cmndline;

	CXErrorCode ec = clang_parseTranslationUnit2(
			idx.clang_idx,
			filename,
			cmndline.data(), cmndline.size(),
			uf_ptr, uf_ptr ? 1 : 0,
			CXTranslationUnit_CacheCompletionResults|CXTranslationUnit_DetailedPreprocessingRecord,
			&clang_tu);

	if (ec != CXError_Success)
	{
		throw std::runtime_error("Error parsing");
	}
}

std::string token::get_kind_name() const
{
	CXTokenKind kind = clang_getTokenKind(*clang_token);
	switch(kind)
	{
		case CXToken_Punctuation: return "punctuation";
		case CXToken_Keyword: return "keyword";
		case CXToken_Identifier: return "identifier";
		case CXToken_Literal : return "literal";
		case CXToken_Comment: return "comment";
	}
	return "dupa";
}

}}
