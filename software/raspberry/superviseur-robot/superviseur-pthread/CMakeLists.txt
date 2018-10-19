set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../superviseur-pthread/bin)

set(superviseur_pthread_FILES ./src/Program.cpp src/Tasks.cpp)

include_directories(./src ../lib)

add_executable(superviseur-pthread ${superviseur_pthread_FILES})
target_link_libraries(superviseur-pthread destijl pthread rt)

