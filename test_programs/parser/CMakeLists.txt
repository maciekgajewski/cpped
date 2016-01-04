

add_executable(parser
	main.cc
)

target_link_libraries(parser

	${CLANG_LIBRARIES}

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
