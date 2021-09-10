cmake_minimum_required (VERSION 3.5)
project(rock-renderer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(TBB REQUIRED)
find_package(ospray REQUIRED)
find_package(rkcommon REQUIRED)

include_directories(
    ${OSPRAY_INCLUDE_DIRS} 
    ${STB_INCLUDES}
    ${RKCOMMON_INCLUDES}
    ${CMAKE_SOURCE_DIR}/include
)

add_subdirectory(external)
add_subdirectory(modules)
