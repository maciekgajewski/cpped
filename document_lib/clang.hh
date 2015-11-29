#pragma once

#include <clang-c/Index.h>

#include <cstdlib>
#include <string>
#include <iterator>

namespace cpped { namespace clang {

// thinc C++ bindings for libclang

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

class file
{
public:
private:
	file(CXFile f) : clang_file(f) {}
	CXFile clang_file;

	friend class translation_unit;
};

class source_location
{
public:
	source_location() : clang_location(clang_getNullLocation()) {}

private:
	source_location(CXTranslationUnit tu, CXFile f, unsigned line, unsigned column)
		: clang_location(clang_getLocation(tu, f, line, column)) {}

	CXSourceLocation clang_location;

	friend class translation_unit;
	friend class source_range;
};

class source_range
{
public:
	source_range() : clang_range(clang_getNullRange()) {}
	source_range(const source_location& b, const source_location& e)
		: clang_range(clang_getRange(b.clang_location, e.clang_location)) {}
private:
	CXSourceRange clang_range;

	friend class translation_unit;
};

class token
{
public:
	token() = delete;

	CXTokenKind get_kind() const { return clang_getTokenKind(*clang_token); }
	std::string get_kind_name() const;

private:
	explicit token(CXToken* t) : clang_token(t) {}
	CXToken* clang_token;

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
		void operator++() { tok.clang_token++; }

	private:
		iterator(CXToken* t) : tok(t) {}
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

	iterator begin() const { return iterator(tokens); }
	iterator end() const { return iterator(tokens + num_tokens); }
private:

	token_list(CXTranslationUnit tu, CXToken* t, unsigned nt)
		: owning_tu(tu), tokens(t), num_tokens(nt) {}

	CXTranslationUnit owning_tu;
	CXToken* tokens = nullptr;
	unsigned num_tokens = 0;

	friend class translation_unit;
};

class translation_unit
{
public:

	// construct by parsing file
	translation_unit(index& idx, const char* filename, const char* unsaved_data, std::size_t unsaved_data_size);
	translation_unit(const translation_unit&) = delete;
	~translation_unit() { clang_disposeTranslationUnit(clang_tu); }

	file get_file(const char* file_name) { return file(clang_getFile(clang_tu, file_name)); }
	file get_file(const std::string& file_name) { return get_file(file_name.c_str()); }
	source_location get_location(const file& file, unsigned line, unsigned column)
	{
		return source_location(clang_tu, file.clang_file, line, column);
	}

	token_list tokenize(const source_range& range)
	{
		CXToken* tokens = nullptr;
		unsigned num_tokens = 0;
		clang_tokenize(clang_tu, range.clang_range, &tokens, &num_tokens);
		return token_list(clang_tu, tokens, num_tokens);
	}

private:

	CXTranslationUnit clang_tu;
};

}}
