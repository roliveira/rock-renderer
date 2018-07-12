cmake_minimum_required (VERSION 3.5)
project(rock-renderer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(external)
add_subdirectory(modules)
