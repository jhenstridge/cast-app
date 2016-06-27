project(cast C CXX)
cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

include(FindPkgConfig)
find_package(Qt5Core REQUIRED)
pkg_check_modules(AVAHI REQUIRED glib-2.0 avahi-core avahi-glib)

add_subdirectory(src/cast)
