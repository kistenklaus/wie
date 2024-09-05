cmake_minimum_required (VERSION 3.11)

include(../cmake/GoogleBenchmark.cmake)
set(CMAKE_CXX_STANDARD 17)

set(BENCH_NAME ${PROJECT_NAME}-bench)
add_executable(${BENCH_NAME} 
  bench.cpp
)

target_compile_options(${BENCH_NAME} PUBLIC -O2)

target_link_libraries(${BENCH_NAME} benchmark::benchmark)
target_link_libraries(${BENCH_NAME} ${PROJECT_NAME})

# Link Shlwapi to the project
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
  target_link_libraries(${BENCH_NAME} Shlwapi)
endif()
