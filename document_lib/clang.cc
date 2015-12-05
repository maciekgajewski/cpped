#include "clang.hh"

#include <stdexcept>
#include <vector>

namespace cpped { namespace clang {

void translation_unit::parse(index& idx, const char* filename, const char* unsaved_data, std::size_t unsaved_data_size)
{
	dispose();

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
			//CXTranslationUnit_CacheCompletionResults|CXTranslationUnit_DetailedPreprocessingRecord|CXTranslationUnit_PrecompiledPreamble,
			clang_defaultEditingTranslationUnitOptions(),
			&clang_tu);

	if (ec != CXError_Success)
	{
		throw std::runtime_error("Error parsing");
	}
}

void translation_unit::reparse(const char* filename, const char* unsaved_data, std::size_t unsaved_data_size)
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

	int error = clang_reparseTranslationUnit(clang_tu, uf_ptr ? 1 : 0, uf_ptr, clang_defaultReparseOptions(clang_tu));

	if (error)
		throw std::runtime_error("Error reparsing translation unit");
}

code_completion_results translation_unit::code_complete_at(const char* filename, unsigned line, unsigned column, const char* unsaved_data, std::size_t unsaved_data_size)
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

	return clang_codeCompleteAt(clang_tu, filename, line, column, uf_ptr, uf_ptr ? 1 : 0, clang_defaultCodeCompleteOptions());
}

void translation_unit::dispose()
{
	if (clang_tu)
	{
		clang_disposeTranslationUnit(clang_tu);
		clang_tu = nullptr;
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

source_location::info source_location::get_location_info() const
{
	info l;
	clang_getFileLocation(clang_location, &l.file.clang_file, &l.line, &l.column, &l.offset);
	return l;
}

}}
