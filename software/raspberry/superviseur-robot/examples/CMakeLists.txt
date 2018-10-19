set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../examples/bin)

set(serialTest_FILES ./src/serialTest.cpp)
set(serverTest_FILES ./src/serverTest.cpp)
include_directories(./src ../lib)

add_executable(serialtest ${serialTest_FILES})
target_link_libraries(serialtest destijl)

add_executable(servertest ${serverTest_FILES})
target_link_libraries(servertest destijl)
