cmake_minimum_required (VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)

set(SANDBOX_NAME ${PROJECT_NAME}-sandbox)

add_executable(${SANDBOX_NAME} 
  main.cpp
)
target_compile_options(${SANDBOX_NAME} PRIVATE "-O0")

target_link_libraries(${SANDBOX_NAME} ${PROJECT_NAME})
