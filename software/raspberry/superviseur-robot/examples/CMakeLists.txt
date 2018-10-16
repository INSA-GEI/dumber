set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../examples/bin)

set(serialTest_FILES ./src/serialTest.cpp)
set(serverTest_FILES ./src/serverTest.cpp)
include_directories(./src ../lib)

add_executable(serialTest ${serialTest_FILES})
target_link_libraries(serialTest destjil)

add_executable(serverTest ${serverTest_FILES})
target_link_libraries(serverTest destjil)
