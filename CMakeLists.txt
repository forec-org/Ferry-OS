cmake_minimum_required(VERSION 3.6)

project(SDOS)
message(STATUS "Project: Software Defined Operating System")
message(STATUS "Project Directory: ${PROJECT_SOURCE_DIR}")

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_COMPILER "clang++")
set(BUILD_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_PATH ${BUILD_OUTPUT_PATH}/lib)
set(EXECUTABLE_OUTPUT_PATH ${BUILD_OUTPUT_PATH}/bin)

find_package(Boost 1.65.1 COMPONENTS system filesystem REQUIRED)

add_compile_options(-std=c++11)

add_subdirectory(src)
add_subdirectory(tests)
