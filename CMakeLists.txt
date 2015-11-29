project(cpped)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14")

find_package(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} COMPONENTS filesystem unit_test_framework)


add_subdirectory(document_lib)
add_subdirectory(cpped)
add_subdirectory(parser)

