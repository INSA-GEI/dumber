cmake_minimum_required(VERSION 3.9)
project(ProjDestjil CXX)

set (ProjDestjil_VERSION_MAJOR 1)
set (ProjDestjil_VERSION_MINOR 0)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

add_subdirectory("lib")
add_subdirectory("examples")
#add_subdirectory("destjil_init")
