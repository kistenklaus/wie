######################################################################abc Blue
# @author      : karl (karlsasssie@gmail.com)
# @file        : CMakeLists
# @created     : Friday Apr 07, 2023 15:39:05 CEST
######################################################################

cmake_minimum_required (VERSION 3.11)
project(wie CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)


SET(BUILD_MODE Debug)
SET(CMAKE_CXX_FLAGS "-Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(${PROJECT_NAME} 
  src/memory/PageAllocator.cpp
  src/memory/pages.cpp
)


target_compile_options(${PROJECT_NAME} PRIVATE -O3 PUBLIC -std=c++23)
target_include_directories(${PROJECT_NAME} PUBLIC src/)


add_subdirectory(bench)
add_subdirectory(test)
add_subdirectory(sandbox)
