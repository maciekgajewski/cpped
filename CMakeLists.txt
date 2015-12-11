project(cpped)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -Wall -Wno-sign-compare")

enable_testing()

include_directories(/usr/lib/llvm-3.7/include)
include_directories(/usr/include/llvm-3.7)
link_directories(/usr/lib/llvm-3.7/lib)

find_package(Boost REQUIRED COMPONENTS system filesystem unit_test_framework)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to generate compilation database

add_subdirectory(document_lib)
add_subdirectory(cpped)
add_subdirectory(test_programs)
add_subdirectory(nct)
