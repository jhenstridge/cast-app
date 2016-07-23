cmake_minimum_required(VERSION 3.0.2)
project(cast C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src/avahi)
add_subdirectory(src/cast)
