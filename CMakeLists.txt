cmake_minimum_required(VERSION 3.20)

project(crafting LANGUAGES C CXX VERSION 0.1)

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Unity Build
set(CMAKE_UNITY_BUILD ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_compile_options(-Wall -Wextra -Werror=return-type)
check_cxx_compiler_flag("-fpattern-matching" HAS_PATTERN_MATCHING)
if(HAS_PATTERN_MATCHING)
    add_compile_options(-fpattern-matching)
endif()

check_cxx_compiler_flag("-fexpansion-statements" HAS_EXPANSION)
if(HAS_EXPANSION)
    add_compile_options(-fexpansion-statements)
endif()

check_cxx_compiler_flag("-freflection" HAS_REFLECTION)
if(HAS_REFLECTION)
    add_compile_options(-freflection)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -fsanitize=address,undefined,leak")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -fsanitize=address,undefined,leak")

set(CMAKE_CXX_FLAGS_RELEASE
    "-O3 -march=native -fno-ident -ftree-vectorize -ffast-math -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,-s -flto"
)

set(CMAKE_CXX_FLAGS_PROFILE "-g -pg")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INCLUDE ${CMAKE_SOURCE_DIR}/src/include)

#################
# PkgConfig     #
#################
find_package(PkgConfig REQUIRED)
pkg_search_module(TBB REQUIRED tbb IMPORTED_TARGET)

#################
# Catch2        #
#################
find_package(Catch2 3 REQUIRED)

#################
# replxx        #
#################
find_package(replxx REQUIRED)

#################
# Boost Library #
#################
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(src)
