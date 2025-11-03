cmake_minimum_required (VERSION 3.11)

include(../cmake/GoogleTest.cmake)
set(CMAKE_CXX_STANDARD 17)

set(TEST_NAME ${PROJECT_NAME}-test)

enable_testing()

add_executable(${TEST_NAME} 
  main.cpp
  container/binary_heap.cpp
  container/kary_heap.cpp
  container/competition/copyable.cpp
  container/competition/immutable.cpp
  container/competition/insertion.cpp
  container/competition/moveable.cpp
  container/competition/random_access.cpp
  container/competition/stack_like.cpp
  container/competition/set_like.cpp
  container/competition/ordered.cpp
  container/competition/queue_like.cpp
  container/competition/range.cpp
  # memory/mallocator.cpp
  # memory/page_allocator.cpp
  # memory/poly_allocator.cpp
  # memory/buddy_allocator.cpp
)


target_compile_options(${TEST_NAME} PRIVATE -g -O0)

target_link_libraries(${TEST_NAME} GTest::gtest)
target_link_libraries(${TEST_NAME} ${PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
