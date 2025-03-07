cmake_minimum_required (VERSION 3.11)

include(../cmake/GoogleTest.cmake)
set(CMAKE_CXX_STANDARD 17)

set(TEST_NAME ${PROJECT_NAME}-test)

enable_testing()

add_executable(${TEST_NAME} 
  counting_allocator.cpp
  pool.cpp
  rc.cpp
  singly_linked_list.cpp
  vec.cpp
)

target_link_libraries(${TEST_NAME} GTest::gtest_main)
target_link_libraries(${TEST_NAME} ${PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
