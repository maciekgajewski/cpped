#include <ncursesw/ncurses.h>

#include <clang-c/Index.h>


int main(int argc, char** argv)
{
	if (argc < 2) return 1;
	::initscr();

	CXIndex clang_idx = clang_createIndex(0, 0);
	CXTranslationUnit clang_tu = nullptr;

//	clang_parseTranslationUnit2(
//			clang_idx,
//			/*argv[1]*/ nullptr,
//			argv, argc, // cmndline
//			nullptr, 0, // unsaved data
//			clang_defaultEditingTranslationUnitOptions(),
//			&clang_tu);

	clang_tu = clang_parseTranslationUnit(
			clang_idx,
			/*argv[1]*/ nullptr,
			argv, argc, // cmndline
			nullptr, 0, // unsaved data
			clang_defaultEditingTranslationUnitOptions());


	::endwin();
}
