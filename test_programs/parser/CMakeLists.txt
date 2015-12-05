# TODO write FindClang
include_directories(/usr/lib/llvm-3.7/include)
include_directories(/usr/include/llvm-3.7)

link_directories(/usr/lib/llvm-3.7/lib)

add_executable(parser
	main.cc
)

target_link_libraries(parser

	LLVM-3.7
	clang-3.7

#clangFrontend
#clangDriver
#clangCodeGen
#clangSema
##clangChecker
#clangAnalysis
#clangRewrite
#clangAST
#clangParse
#clangLex
#clangBasic
#clangARCMigrate
#clangEdit
#clangFrontendTool
#clangRewrite
#clangSerialization
#clangTooling
#clangStaticAnalyzerCheckers
#clangStaticAnalyzerCore
#clangStaticAnalyzerFrontend
#clangSema
##clangRewriteCore

#clangRewriteFrontend
#clangASTMatchers
#clangToolingCore


)
