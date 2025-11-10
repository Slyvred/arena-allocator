cmake_minimum_required(VERSION 4.1)
project(arena)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_FILES
    src/main.cpp
    src/arena.hpp
    src/logger.hpp
    src/arena_list.hpp
)

add_executable(arena ${SOURCE_FILES})

# ----------------------------
# Build Type / Release Settings
# ----------------------------
# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
