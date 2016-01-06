#pragma once

#include <clang-c/Index.h>
#include <clang-c/CXCompilationDatabase.h>

#include <boost/filesystem.hpp>

#include <cstdlib>
#include <string>
#include <iterator>
#include <vector>
#include <cassert>

namespace cpped { namespace clang {

// thin C++ bindings for libclang

class string
{
public:
	~string() { clang_disposeString(clang_string); }

	const char* c_str() const { return clang_getCString(clang_string); }

private:
	string(const CXString s) : clang_string(s) {}
	CXString clang_string;

	friend class cursor;
	friend class completion_string;
	friend class code_completion_result;
	friend class compile_command;
	friend class source_file;
	friend class diagnostic;
};

class index
{
public:
	index(int excludeDeclarationsFromPCH,int displayDiagnostics)
		: clang_idx(clang_createIndex(excludeDeclarationsFromPCH, displayDiagnostics)) {}
	index(const index&) = delete;
	index(index&& o) : clang_idx(o.clang_idx) { o.clang_idx = nullptr; }
	~index() { if (!clang_idx) clang_disposeIndex(clang_idx); }

	index& operator=(index&& o)
	{
		if (clang_idx)
			clang_disposeIndex(clang_idx);
		clang_idx = o.clang_idx;
		o.clang_idx = nullptr;
		return *this;
	}
private:

	CXIndex clang_idx = nullptr;
	friend class translation_unit;
};

class source_file
{
public:
	source_file() = default;
	string get_name() const { return clang_getFileName(file_); }
	bool is_null() const { return file_ == nullptr; }
private:
	source_file(CXFile f) : file_(f) {}
	CXFile file_ = nullptr;

	friend class translation_unit;
	friend class source_location;
	friend class cursor;
};

class source_location
{
public:
	struct info
	{
		unsigned line;
		unsigned column;
		unsigned offset;
		source_file file;
	};

	source_location() : clang_location(clang_getNullLocation()) {}

	info get_location_info() const;
	bool is_null() const { return clang_equalLocations(clang_location, clang_getNullLocation()); }

private:

	source_location(const CXSourceLocation& sl) : clang_location(sl) {}

	CXSourceLocation clang_location;

	friend class translation_unit;
	friend class source_range;
	friend class token_list;
};

class source_range
{
public:
	source_range() : clang_range(clang_getNullRange()) {}
	source_range(const source_location& b, const source_location& e)
		: clang_range(clang_getRange(b.clang_location, e.clang_location)) {}

	source_location get_start() const { return clang_getRangeStart(clang_range); }
	source_location get_end() const { return clang_getRangeEnd(clang_range); }

private:
	source_range(const CXSourceRange& rng) : clang_range(rng) {}
	CXSourceRange clang_range;

	friend class translation_unit;
	friend class token_list;
	friend class diagnostic;
};

inline std::ostream& operator<<(std::ostream& s, const string& cs)
{
	if (cs.c_str())
		s << cs.c_str();
	return s;
}

class cursor
{
public:

	CXCursorKind get_kind() const { return clang_getCursorKind(clang_cursor); }
	string get_kind_as_string() const { return clang_getCursorKindSpelling(clang_getCursorKind(clang_cursor)); }

	using visitor_return_type =  CXChildVisitResult;
	// function_t: CXChildVisitResult(const cursor& visited_cursor, const cursor& parent)
	template<typename function_t>
	void visit_children(function_t fun) const
	{
		CXCursorVisitor visitor = [](CXCursor visited_cursor, CXCursor parent, CXClientData cd) -> CXChildVisitResult
		{
			function_t& fref = *reinterpret_cast<function_t*>(cd);

			return fref(cursor(visited_cursor), cursor(parent));
		};

		clang_visitChildren(clang_cursor, visitor, reinterpret_cast<CXClientData>(&fun));
	}

	cursor get_lexical_parent() const { return clang_getCursorLexicalParent(clang_cursor); }
	cursor get_semantic_parent() const { return clang_getCursorSemanticParent(clang_cursor); }
	source_file get_included_file() const { return clang_getIncludedFile(clang_cursor); }

private:
	cursor(const CXCursor& c) : clang_cursor(c) {}
	CXCursor clang_cursor;

	friend class token;
	friend class translation_unit;
};

class token
{
public:
	token() = delete;

	CXTokenKind get_kind() const { return clang_getTokenKind(*clang_token); }
	std::string get_kind_name() const;

	bool has_associated_cursor() const { return associated_cursor != nullptr; }

	cursor get_associated_cursor() const
	{
		if (associated_cursor)
			return cursor(*associated_cursor);
		else
			throw std::logic_error("Token not annotated");
	}

private:
	explicit token(CXToken* t, const CXCursor* c)  : clang_token(t), associated_cursor(c) {}
	CXToken* clang_token;
	const CXCursor* associated_cursor = nullptr;

	friend class token_list;

};

class token_list
{
public:

	class iterator : public std::iterator<std::random_access_iterator_tag, token>
	{
	public:

		const token& operator*() const { return tok; }
		const token* operator->()const { return &tok; }
		bool operator==(const iterator& o) const { return tok.clang_token == o.tok.clang_token; }
		bool operator!=(const iterator& o) const { return tok.clang_token != o.tok.clang_token; }
		void operator++() { tok.clang_token++; if(tok.associated_cursor) tok.associated_cursor++; }

	private:
		iterator(CXToken* t, const CXCursor* c) : tok(t, c) {}
		token tok;

		friend class token_list;
	};

	~token_list() { if (tokens) clang_disposeTokens(owning_tu, tokens, num_tokens); }
	token_list(token_list&& o)
	{
		owning_tu = o.owning_tu;
		tokens = o.tokens;
		num_tokens = o.num_tokens;
		o.tokens = nullptr;
		o.num_tokens = 0;
	}
	token_list() = delete;
	token_list(const token_list&) = delete;

	size_t size() const { return num_tokens; }

	iterator begin() const
	{
		if (associated_cursors.empty())
		{
			return iterator(tokens, nullptr);
		}
		else
		{
			assert(associated_cursors.size() == num_tokens);
			return iterator(tokens,	associated_cursors.data());
		}
	}
	iterator end() const
	{
		if (associated_cursors.empty())
		{
			return iterator(tokens + num_tokens, nullptr);
		}
		else
		{
			assert(associated_cursors.size() == num_tokens);
			return iterator(tokens + num_tokens, associated_cursors.data() + num_tokens);
		}
	}

	source_location get_token_location(const token& t) const
	{
		return source_location(clang_getTokenLocation(owning_tu, *t.clang_token));
	}

	source_range get_token_extent(const token& t) const
	{
		return source_range(clang_getTokenExtent(owning_tu, *t.clang_token));
	}

	void annotate_tokens()
	{
		associated_cursors.resize(num_tokens);
		clang_annotateTokens(owning_tu, tokens, num_tokens, associated_cursors.data());
	}

private:

	token_list(CXTranslationUnit tu, CXToken* t, unsigned nt)
		: owning_tu(tu), tokens(t), num_tokens(nt) {}

	CXTranslationUnit owning_tu;
	CXToken* tokens = nullptr;
	unsigned num_tokens = 0;
	std::vector<CXCursor> associated_cursors;

	friend class translation_unit;
};

class completion_string
{
public:

	unsigned get_num_chunks() const { return clang_getNumCompletionChunks(clang_completion_string); }
	string get_chunk_text(unsigned idx) { return clang_getCompletionChunkText(clang_completion_string, idx); }
	CXCompletionChunkKind get_chunk_kind(unsigned idx) const { return clang_getCompletionChunkKind(clang_completion_string, idx); }
	unsigned get_priority() const { return clang_getCompletionPriority(clang_completion_string); }
	string get_brief_comment() const { return  clang_getCompletionBriefComment(clang_completion_string); }

private:

	completion_string(CXCompletionString s) : clang_completion_string(s) {}
	CXCompletionString clang_completion_string;

	friend class code_completion_result;
};

const char* completion_chunk_kind_to_str(CXCompletionChunkKind kind);

class code_completion_result
{
public:

	CXCursorKind get_cursor_kind() const { return result->CursorKind; }
	string get_cursor_kind_as_string() const { return clang_getCursorKindSpelling(result->CursorKind); }

	completion_string get_completion_string() const { return completion_string(result->CompletionString); }

private:

	code_completion_result(CXCompletionResult* r) : result(r) {}
	CXCompletionResult* result;

	friend class code_completion_results;
};

class code_completion_results
{
public:

	class iterator : public std::iterator<std::random_access_iterator_tag, code_completion_result>
	{
	public:
		const code_completion_result& operator*() const { return result; }
		const code_completion_result* operator->()const { return &result; }
		bool operator==(const iterator& o) const { return result.result == o.result.result; }
		bool operator!=(const iterator& o) const { return result.result != o.result.result; }
		void operator++() { result.result++; }
	private:
		iterator(CXCompletionResult* r) : result(r) {}
		code_completion_result result;

		friend class code_completion_results;
	};

	~code_completion_results() { dispose(); }

	bool is_null() const { return results == nullptr; }

	iterator begin() const { return iterator(results->Results); }
	iterator end() const { return iterator(results->Results + results->NumResults); }
	std::size_t size() const { return results->NumResults; }

private:

	code_completion_results(CXCodeCompleteResults* cr) { dispose(); results = cr; }

	void dispose()
	{
		if (results)
			clang_disposeCodeCompleteResults(results);
		results = nullptr;
	}

	CXCodeCompleteResults* results = nullptr;

	friend class translation_unit;
};

class diagnostic
{
public:

	~diagnostic() { dispose(); }

	// options: CXDiagnosticDisplayOptions
	string format(unsigned opts = 0) const { return clang_formatDiagnostic(diag_, opts); }

	CXDiagnosticSeverity get_severity() const { return clang_getDiagnosticSeverity(diag_); }

	std::size_t get_num_ranges() const { return clang_getDiagnosticNumRanges(diag_); }
	source_range get_range(unsigned idx) { return clang_getDiagnosticRange(diag_, idx); }

private:

	diagnostic(CXDiagnostic d) : diag_(d) {}

	void dispose()
	{
		if (diag_)
		{
			clang_disposeDiagnostic(diag_);
			diag_ = nullptr;
		}
	}

	CXDiagnostic diag_ = nullptr;

	friend class diagnostic_set;
};

class diagnostic_set
{
public:
	~diagnostic_set() { dispose(); }

	std::size_t size() const { return clang_getNumDiagnosticsInSet(diag_set_); }

	diagnostic operator[](unsigned i) const { return clang_getDiagnosticInSet(diag_set_, i); }
private:

	diagnostic_set(CXDiagnosticSet s) : diag_set_(s) {}

	void dispose()
	{
		if (diag_set_)
		{
			clang_disposeDiagnosticSet(diag_set_);
			diag_set_ = nullptr;
		}
	}
	CXDiagnosticSet diag_set_ = nullptr;

	friend class translation_unit;
};

class translation_unit
{
public:

	// construct by parsing file
	translation_unit() = default;
	translation_unit(const translation_unit&) = delete;
	~translation_unit() { dispose(); }

	void parse(index& idx, const char* filename, const char* unsaved_data, std::size_t unsaved_data_size, const std::vector<const char*> cmdline, unsigned parsingOptions);
	void reparse(const char* filename, const char* unsaved_data, std::size_t unsaved_data_size);

	void parse(index& idx, const char* filename, const std::vector<CXUnsavedFile>& unsaved_data, const std::vector<const char*> cmdline, unsigned opts);
	void reparse(const std::vector<CXUnsavedFile>& unsaved_data);

	source_file get_file(const char* file_name) const { return source_file(clang_getFile(clang_tu, file_name)); }
	source_file get_file(const std::string& file_name) const { return get_file(file_name.c_str()); }
	source_location get_location(const source_file& file, unsigned line, unsigned column) const
	{
		source_location location(clang_getLocation(clang_tu, file.file_, line, column));
		if (location.is_null())
		{
			throw std::runtime_error("no such location");
		}
		return location;
	}

	source_location get_location_for_offset(const source_file& file, unsigned offset) const
	{
		source_location location(clang_getLocationForOffset(clang_tu, file.file_, offset));
		if (location.is_null())
		{
			throw std::runtime_error("no such location");
		}
		return location;
	}

	token_list tokenize(const source_range& range) const
	{
		CXToken* tokens = nullptr;
		unsigned num_tokens = 0;
		clang_tokenize(clang_tu, range.clang_range, &tokens, &num_tokens);
		return token_list(clang_tu, tokens, num_tokens);
	}

	code_completion_results code_complete_at(const char* filename, unsigned line, unsigned column, const char* unsaved_data, std::size_t unsaved_data_size);
	code_completion_results code_complete_at(const char* filename, unsigned line, unsigned column, const std::vector<CXUnsavedFile>& unsaved_data);

	bool is_null() const { return clang_tu == nullptr; }
	void dispose();

	cursor get_cursor() const { return clang_getTranslationUnitCursor(clang_tu); }

	diagnostic_set get_diagnostics() const { return clang_getDiagnosticSetFromTU(clang_tu); }

	static unsigned full_parsing_options();

private:

	CXTranslationUnit clang_tu = nullptr;
};

class compile_command
{
public:

	unsigned size() const { return clang_CompileCommand_getNumArgs(cc_); }
	string get_arg(unsigned idx) const { return clang_CompileCommand_getArg(cc_, idx); }

#if CINDEX_VERSION_MINOR > 31
	string get_file_name() const { return clang_CompileCommand_getFilename(cc_); }
#endif
	string get_dir() const { return clang_CompileCommand_getDirectory(cc_); }

private:
	compile_command(CXCompileCommand cc) : cc_(cc) {}

	CXCompileCommand cc_ = nullptr;

	friend class compile_commands;
};

class compile_commands
{
public:
	compile_commands(const compile_commands&) = delete;
	compile_commands(compile_commands&& o) : cc_(o.cc_)	{ o.cc_ = nullptr; }

	~compile_commands()
	{
		if (cc_)
			clang_CompileCommands_dispose(cc_);
	}

	unsigned size() const { return clang_CompileCommands_getSize(cc_); }
	compile_command get_command(unsigned idx) const { return clang_CompileCommands_getCommand(cc_, idx); }

	bool is_null() const { return cc_ == nullptr; }

private:
	compile_commands(CXCompileCommands cc) : cc_(cc) {}

	CXCompileCommands cc_ = nullptr;

	friend class compilation_database;
};

class compilation_database
{
public:
	compilation_database(const boost::filesystem::path& directory)
	{
		CXCompilationDatabase_Error err = CXCompilationDatabase_CanNotLoadDatabase;
		cd_ = clang_CompilationDatabase_fromDirectory(directory.string().c_str(), &err);
		if (err == CXCompilationDatabase_CanNotLoadDatabase)
		{
			throw std::runtime_error("Unable to load compilation database");
		}
	}
	compilation_database(const compilation_database&) = delete;

	~compilation_database()
	{
		if (cd_)
			clang_CompilationDatabase_dispose(cd_);
	}

	compile_commands get_all_compile_commands() const
	{
		return clang_CompilationDatabase_getAllCompileCommands(cd_);
	}

	compile_commands get_compile_commands_for_file(const boost::filesystem::path& file) const
	{
		assert(file.is_absolute());
		return clang_CompilationDatabase_getCompileCommands(cd_, file.string().c_str());
	}

private:

	CXCompilationDatabase cd_ = nullptr;
};


}}
