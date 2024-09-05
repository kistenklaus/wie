######################################################################
# @author      : karl (karlsasssie@gmail.com)
# @file        : CMakeLists
# @created     : Friday Apr 07, 2023 15:39:05 CEST
######################################################################

cmake_minimum_required (VERSION 3.11)
project(wie CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)


add_library(${PROJECT_NAME} 
  src/lib.cpp
  src/linked_pool.h
)

target_compile_options(${PROJECT_NAME} PRIVATE -O3)

target_include_directories(${PROJECT_NAME} PUBLIC src/)


add_subdirectory(bench)
add_subdirectory(test)
add_subdirectory(sandbox)
