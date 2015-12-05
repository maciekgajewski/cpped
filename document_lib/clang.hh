#pragma once

#include <clang-c/Index.h>

#include <cstdlib>
#include <string>
#include <iterator>
#include <vector>
#include <cassert>

namespace cpped { namespace clang {

// thin C++ bindings for libclang

class index
{
public:
	index(int excludeDeclarationsFromPCH,int displayDiagnostics)
		: clang_idx(clang_createIndex(excludeDeclarationsFromPCH, displayDiagnostics)) {}
	index(const index&) = delete;
	~index() { clang_disposeIndex(clang_idx); }
private:

	CXIndex clang_idx;
	friend class translation_unit;
};

class source_file
{
public:
	source_file() : clang_file(nullptr) {}
private:
	source_file(CXFile f) : clang_file(f) {}
	CXFile clang_file;

	friend class translation_unit;
	friend class source_location;
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
	friend class source_range;
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
};

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
};

inline std::ostream& operator<<(std::ostream& s, const string& cs)
{
	return s << cs.c_str();
}

class cursor
{
public:

	CXCursorKind get_kind() const { return clang_getCursorKind(clang_cursor); }
	string get_kind_as_string() const { return clang_getCursorKindSpelling(clang_getCursorKind(clang_cursor)); }

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

private:
	cursor(const CXCursor& c) : clang_cursor(c) {}
	CXCursor clang_cursor;

	friend class token;
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

private:

	completion_string(CXCompletionString s) : clang_completion_string(s) {}
	CXCompletionString clang_completion_string;

	friend class code_completion_result;
};

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

private:

	code_completion_results(CXCodeCompleteResults* cr)  { dispose(); results = cr; }

	void dispose()
	{
		if (results)
			clang_disposeCodeCompleteResults(results);
		results = nullptr;
	}

	CXCodeCompleteResults* results = nullptr;

	friend class translation_unit;
};

class translation_unit
{
public:

	// construct by parsing file
	translation_unit() = default;
	translation_unit(const translation_unit&) = delete;
	~translation_unit() { dispose(); }

	void parse(index& idx, const char* filename, const char* unsaved_data, std::size_t unsaved_data_size);
	void reparse(const char* filename, const char* unsaved_data, std::size_t unsaved_data_size);

	source_file get_file(const char* file_name) { return source_file(clang_getFile(clang_tu, file_name)); }
	source_file get_file(const std::string& file_name) { return get_file(file_name.c_str()); }
	source_location get_location(const source_file& file, unsigned line, unsigned column)
	{
		source_location location(clang_getLocation(clang_tu, file.clang_file, line, column));
		if (location.is_null())
		{
			throw std::runtime_error("no such location");
		}
		return location;
	}

	source_location get_location_for_offset(const source_file& file, unsigned offset)
	{
		source_location location(clang_getLocationForOffset(clang_tu, file.clang_file, offset));
		if (location.is_null())
		{
			throw std::runtime_error("no such location");
		}
		return location;
	}

	token_list tokenize(const source_range& range)
	{
		CXToken* tokens = nullptr;
		unsigned num_tokens = 0;
		clang_tokenize(clang_tu, range.clang_range, &tokens, &num_tokens);
		return token_list(clang_tu, tokens, num_tokens);
	}

	code_completion_results code_complete_at(const char* filename, unsigned line, unsigned column, const char* unsaved_data, std::size_t unsaved_data_size);

	bool is_null() const { return clang_tu == nullptr; }

private:

	void dispose();

	CXTranslationUnit clang_tu = nullptr;
};

}}
