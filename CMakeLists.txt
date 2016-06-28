project(cast C CXX)
cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src/cast)
