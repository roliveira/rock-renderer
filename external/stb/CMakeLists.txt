cmake_minimum_required(VERSION 2.8.8)
project(stb_builder CXX)

include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    stb
    PREFIX ${CMAKE_BINARY_DIR}/stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(stb source_dir)

set(STB_MODULE_PATH "${source_dir}" CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
set(STB_INCLUDES     ${source_dir}  CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
