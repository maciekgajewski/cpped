project(cpped)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -Wall -Wno-sign-compare")

enable_testing()

# Debian provided
set(CLANG_INCLUDE_DIRS /usr/lib/llvm-3.7/include /usr/include/llvm-3.7)
set(CLANG_LINK_DIRS /usr/lib/llvm-3.7/lib)
set(CLANG_LIBRARIES	LLVM-3.7 clang-3.7)

# local debug build
#set(CLANG_INCLUDE_DIRS /home/maciek/workspace/llvm/tools/clang/include)
#set(CLANG_LINK_DIRS /home/maciek/workspace/build/llvm-debug/lib)
#set(CLANG_LIBRARIES	clang)

include_directories(${CLANG_INCLUDE_DIRS})
link_directories(${CLANG_LINK_DIRS})

# cursome ncurses
#INCLUDE_DIRECTORIES(/home/maciek/workspace/build/ncurses-debug/include)
#LINK_DIRECTORIES(/home/maciek/workspace/build/ncurses-debug/lib)

find_package(Boost REQUIRED COMPONENTS system filesystem filesystem)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to generate compilation database

add_subdirectory(clang_lib)
add_subdirectory(document_lib)
add_subdirectory(cpped)
add_subdirectory(test_programs)
add_subdirectory(nct)
add_subdirectory(backend_lib)
