cmake_minimum_required(VERSION 3.9)
project(stress)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ../../src/robot.cpp ../../src/robot.h)
add_executable(stress ${SOURCE_FILES})