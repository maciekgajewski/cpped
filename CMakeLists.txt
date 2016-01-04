project(cpped)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -Wall -Wno-sign-compare")

enable_testing()


option(CLANG_ROOT "Clang installation dir")

if(DEFINED CLANG_ROOT)
	set(CLANG_INCLUDE_DIRS ${CLANG_ROOT}/include)
	set(CLANG_LINK_DIRS ${CLANG_ROOT}/lib)
	set(CLANG_LIBRARIES    clang)
else()
# Debian provided
	set(CLANG_INCLUDE_DIRS /usr/lib/llvm-3.7/include /usr/include/llvm-3.7)
	set(CLANG_LINK_DIRS /usr/lib/llvm-3.7/lib)
	set(CLANG_LIBRARIES	LLVM-3.7 clang-3.7)
endif()

include_directories(${CLANG_INCLUDE_DIRS})
link_directories(${CLANG_LINK_DIRS})

# cursome ncurses
#INCLUDE_DIRECTORIES(/home/maciek/workspace/build/ncurses-debug/include)
#LINK_DIRECTORIES(/home/maciek/workspace/build/ncurses-debug/lib)

find_package(Boost COMPONENTS system filesystem program_options date_time)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

if((NOT Boost_FILESYSTEM_FOUND) OR (NOT Boost_PROGRAM_OPTIONS_FOUND) OR (NOT Boost_DATE_TIME_FOUND))
	message(FATAL_ERROR "Required boost components not found")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to generate compilation database

option(ENABLE_LOGGING "enables logging facility" 0)
if(ENABLE_LOGGING)
	add_definitions(-DENABLE_LOGGING)
endif()

add_subdirectory(clang_lib)
add_subdirectory(document_lib)
add_subdirectory(cpped)
add_subdirectory(test_programs)
add_subdirectory(nct)
add_subdirectory(backend_lib)
