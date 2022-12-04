
cmake_minimum_required (VERSION 3.16)

project (tcp_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} main.cpp)

if(NOT WIN32)
  target_link_libraries(${PROJECT_NAME} pthread)
endif()