cmake_minimum_required (VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)

set(SANDBOX_NAME ${PROJECT_NAME}-sandbox)

add_executable(${SANDBOX_NAME} 
  main.cpp
)

target_link_libraries(${SANDBOX_NAME} ${PROJECT_NAME})
