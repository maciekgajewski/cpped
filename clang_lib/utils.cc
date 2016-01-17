#include "utils.hh"

namespace cpped { namespace clang {

namespace fs = boost::filesystem;

boost::container::flat_set<boost::filesystem::path> get_includes(clang::translation_unit& tu)
{
	boost::container::flat_set<fs::path> includes;
	includes.reserve(32);

	clang::cursor cursor = tu.get_cursor();
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

	return includes;
}

}}
