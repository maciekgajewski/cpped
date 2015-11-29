
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <chrono>

#include <clang-c/Index.h>

template<typename PTRTYPE>
class resource_pointer_wrapper
{
public:
	resource_pointer_wrapper(const resource_pointer_wrapper&) = delete;
	resource_pointer_wrapper(PTRTYPE p) : pointer(p) {}
	operator PTRTYPE () { return pointer; }

protected:

	PTRTYPE pointer;
};

struct translation_unit : public resource_pointer_wrapper<CXTranslationUnit>
{
	using resource_pointer_wrapper::resource_pointer_wrapper;
	~translation_unit() { clang_disposeTranslationUnit(pointer); }
};

template <typename PTRTYPE, typename DELETERTYPE>
class resource_pointer_wrapper2
{
public:
	resource_pointer_wrapper2() = default;
	resource_pointer_wrapper2(PTRTYPE ptr) : pointer(ptr) {}
	resource_pointer_wrapper2(const resource_pointer_wrapper2&) = delete;
	~resource_pointer_wrapper2() { dispose(); }
	resource_pointer_wrapper2(resource_pointer_wrapper2&& o) { std::swap(pointer, o.pointer); }

	resource_pointer_wrapper2& operator=(PTRTYPE ptr)
	{
		if (ptr)
			dispose();
		pointer = ptr;
	}

	operator PTRTYPE () { return pointer; }
	PTRTYPE operator->() { return pointer; }
	operator const PTRTYPE () const { return pointer; }
	const PTRTYPE operator->() const { return pointer; }

private:
	void dispose()
	{
		if(pointer)
			DELETERTYPE()(pointer);
	}
	PTRTYPE pointer = nullptr;
};

#define MAKE_POINTER_WRAPPER(name, ptrtype, dispose_function)\
	namespace internal { struct __##name##_disposer { void operator()(ptrtype p) { dispose_function(p); } }; } \
	using name = resource_pointer_wrapper2<ptrtype, internal::__##name##_disposer>;

//using diagnostics_set = resource_pointer_wrapper2<CXDiagnosticSet, decltype([](CXDiagnosticSet* s){ clang_disposeDiagnosticSet(s); })>;
MAKE_POINTER_WRAPPER(diagnostic_set, CXDiagnosticSet, clang_disposeDiagnosticSet)
MAKE_POINTER_WRAPPER(diagnostic, CXDiagnostic, clang_disposeDiagnostic)

template<typename STRUCT>
class resource_struct_wrapper
{
public:
	resource_struct_wrapper(const resource_struct_wrapper&) = delete;
	resource_struct_wrapper(const STRUCT& s) : resource(s) {}

	STRUCT* operator->() { return &resource; }
	const STRUCT* operator->() const { return &resource; }

protected:
	STRUCT resource;
};

struct resource_usage : public resource_struct_wrapper<CXTUResourceUsage>
{
	using resource_struct_wrapper::resource_struct_wrapper;
	~resource_usage() { clang_disposeCXTUResourceUsage(resource); }
};

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		throw std::runtime_error("filename param missing");
	}
	const char* filename = argv[1];
	std::cout << "Parsing file: " << filename << std::endl;

//	std::fstream file(filename, std::ios_base::in);
//	if (file.fail())
//		throw std::runtime_error("Failed to load file");


//	std::vector<std::string> data;
//	std::string line;
//	while(!file.eof())
//	{
//		line.clear();
//		std::getline(file, line);
//		data.push_back(line);
//	}

//	llvm::Twine code_twine;
//	for(const std::string& s : data)
//	{
//		code_twine.con
//	}

	using namespace std::literals::chrono_literals;

	// TODO
	CXIndex index = clang_createIndex(0, 0);
	std::vector<const char*> cmndline;
	CXTranslationUnit tu_raw;
	auto parse_start_time = std::chrono::high_resolution_clock::now();
	CXErrorCode ec = clang_parseTranslationUnit2(index, filename, cmndline.data(), cmndline.size(), nullptr, 0, CXTranslationUnit_CacheCompletionResults, &tu_raw);
	translation_unit tu(tu_raw);

	auto parse_end_time = std::chrono::high_resolution_clock::now();
	if (ec != CXError_Success)
	{
		std::cout << "Error parsing, code=" << ec << std::endl;
		return 1;
	}
	std::cout << "Parsed OK, duration= " << (parse_end_time - parse_start_time)/1ms << " ms"  << std::endl;

	// get resource usage
	std::cout << "Resources:" << std::endl;
	resource_usage usage(clang_getCXTUResourceUsage(tu));
	for(unsigned i = 0; i < usage->numEntries; ++i)
	{
		CXTUResourceUsageEntry& entry = usage->entries[i];
		std::cout << " * " << clang_getTUResourceUsageName(entry.kind) << " : " << entry.amount << std::endl;
	}

	// get diagnostics
	std::cout << "Disagnostics: " << std::endl;
	unsigned num_diag= clang_getNumDiagnostics(tu);
	for(unsigned i = 0; i < num_diag; ++i)
	{
		diagnostic d = clang_getDiagnostic(tu, i);
		CXString s = clang_getDiagnosticSpelling(d);

		std::cout << " * " << clang_getCString(s) << std::endl;
		clang_disposeString(s);
	}


}

