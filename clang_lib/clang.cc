#include "clang.hh"

#include <stdexcept>
#include <vector>

namespace cpped { namespace clang {

void translation_unit::parse(index& idx, const char* filename, const char* unsaved_data, std::size_t unsaved_data_size, const std::vector<const char*> cmdline)
{
	std::vector<CXUnsavedFile> unsaved_data_vec;
	if (unsaved_data)
	{
		unsaved_data_vec.push_back(
			CXUnsavedFile{filename, unsaved_data, unsaved_data_size});
	}
	parse(idx, filename, unsaved_data_vec, cmdline);
}

void translation_unit::parse(index& idx, const char* filename, const std::vector<CXUnsavedFile>& unsaved_data, const std::vector<const char*> cmdline)
{
	dispose();

	unsigned parsingOptions = CXTranslationUnit_CacheCompletionResults
		 | CXTranslationUnit_PrecompiledPreamble
		 | CXTranslationUnit_IncludeBriefCommentsInCodeCompletion
		 | CXTranslationUnit_DetailedPreprocessingRecord;

	CXErrorCode ec = clang_parseTranslationUnit2(
			idx.clang_idx,
			filename,
			cmdline.data(), cmdline.size(),
			const_cast<CXUnsavedFile*>(unsaved_data.data()), unsaved_data.size(),
			parsingOptions,
			&clang_tu);

	if (ec != CXError_Success)
	{
		throw std::runtime_error("Error parsing");
	}
}

void translation_unit::reparse(const char* filename, const char* unsaved_data, std::size_t unsaved_data_size)
{
	std::vector<CXUnsavedFile> unsaved_data_vec;
	if (unsaved_data)
	{
		unsaved_data_vec.push_back(
			CXUnsavedFile{filename, unsaved_data, unsaved_data_size});
	}
	reparse(unsaved_data_vec);
}

void translation_unit::reparse(const std::vector<CXUnsavedFile>& unsaved_data)
{
	int error = clang_reparseTranslationUnit(clang_tu, unsaved_data.size(), const_cast<CXUnsavedFile*>(unsaved_data.data()), clang_defaultReparseOptions(clang_tu));

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


	CXCodeCompleteResults* results = clang_codeCompleteAt(clang_tu, filename, line, column, uf_ptr, uf_ptr ? 1 : 0,
			/*clang_defaultCodeCompleteOptions()|CXCodeComplete_IncludeBriefComments*/CXCodeComplete_IncludeMacros|CXCodeComplete_IncludeCodePatterns);
	if (!results)
		throw std::runtime_error("Code completion failed");

	return results;
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
	clang_getFileLocation(clang_location, &l.file.file_, &l.line, &l.column, &l.offset);
	return l;
}

const char* completion_chunk_kind_to_str(CXCompletionChunkKind kind)
{
	switch(kind)
	{
		case CXCompletionChunk_Optional: return "Optional";
		case CXCompletionChunk_TypedText: return "TypedText";
		case CXCompletionChunk_Text: return "Text";
		case CXCompletionChunk_Placeholder: return "Placeholder";
		case CXCompletionChunk_Informative: return "Informative";
		case CXCompletionChunk_CurrentParameter: return "CurrentParameter";
		case CXCompletionChunk_LeftParen: return "LeftParen";
		case CXCompletionChunk_RightParen: return "RightParen";
		case CXCompletionChunk_LeftBracket: return "LeftBracket";
		case CXCompletionChunk_RightBracket: return "RightBracket";
		case CXCompletionChunk_LeftBrace: return "LeftBrace";
		case CXCompletionChunk_RightBrace: return "RightBrace";
		case CXCompletionChunk_LeftAngle: return "LeftAngle";
		case CXCompletionChunk_RightAngle: return "RightAngle";
		case CXCompletionChunk_Comma: return "Comma";
		case CXCompletionChunk_ResultType: return "ResultType";
		case CXCompletionChunk_Colon: return "Colon";
		case CXCompletionChunk_SemiColon: return "SemiColon";
		case CXCompletionChunk_Equal: return "Equal";
		case CXCompletionChunk_HorizontalSpace: return "HorizontalSpace";
		case CXCompletionChunk_VerticalSpace: return "VerticalSpace";
	};

	return "?";
}

}}
