# TODO write FindClang
include_directories(/usr/lib/llvm-3.7/include)
include_directories(/usr/include/llvm-3.7)

add_library(clang++
	index.cxx
	translationUnit.cxx
	translationUnitCache.cxx
	sourceLocation.cxx
	cursor.cxx
)

target_include_directories(document_lib INTERFACE ..)

target_link_libraries(clang++

	LLVM-3.7
	clang-3.7
)
