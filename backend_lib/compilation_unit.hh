#pragma once

#include "clang_lib/clang.hh"

#include <boost/filesystem.hpp>

namespace cpped { namespace backend {

// Translation unit
class compilation_unit
{
public:

	compilation_unit(const boost::filesystem::path& path, clang::index& idx);

	bool is_parsed() const { return !translation_unit_.is_null(); }
	void parse();

	template<typename Container>
	void set_compilation_commands(const Container& in)
	{
		compilation_commands_.assign(std::begin(in), std::end(in));
	}

private:

	boost::filesystem::path path_;
	clang::index& index_;
	clang::translation_unit translation_unit_;
	std::vector<std::string> compilation_commands_;
};

}}
