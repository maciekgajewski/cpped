project(cpped)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -Wall -Wno-sign-compare")

include_directories(/usr/lib/llvm-3.7/include)
include_directories(/usr/include/llvm-3.7)
link_directories(/usr/lib/llvm-3.7/lib)

find_package(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} COMPONENTS filesystem unit_test_framework)


add_subdirectory(document_lib)
add_subdirectory(cpped)
add_subdirectory(parser)
