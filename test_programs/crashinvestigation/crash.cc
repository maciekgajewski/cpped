#include <ncursesw/ncurses.h>

#include <clang-c/Index.h>


int main(int argc, char** argv)
{
	if (argc < 2) return 1;
	CXIndex clang_idx = clang_createIndex(0, 0);

	::initscr();
	//::trace(TRACE_MAXIMUM);

	//::_tracef("before calling parse");

	CXTranslationUnit clang_tu2 = clang_parseTranslationUnit(
			clang_idx,
			/*argv[1]*/ nullptr,
			argv, argc, // cmndline
			nullptr, 0, // unsaved data
			clang_defaultEditingTranslationUnitOptions());

	(void)clang_tu2;

	//::_tracef("after calling parse");


	::endwin();
}
