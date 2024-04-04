cmake_minimum_required(VERSION 3.14.0)
project(superviseur-pthreads VERSION 0.1.0 LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENCV REQUIRED opencv4)

set(THREADS_PREFER_PTHREAD_FLAG ON)
pkg_check_modules(Threads REQUIRED)

include(CTest)
enable_testing()

add_definitions( -D__WITH_PTHREAD__ )
add_definitions( -D__WITH_ARUCO__ ) 
add_definitions( -D__FOR_PC__ )

include_directories(lib lib/base64)

add_executable(superviseur-pthreads 
                main.cpp 
                tasks_pthread.cpp 
                lib/camera.cpp 
                lib/commonitor.cpp
                lib/img.cpp
                lib/messages.cpp
                lib/comrobot.cpp
                lib/base64/base64.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(superviseur-pthreads ${OPENCV_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(superviseur-pthreads PUBLIC ${OPENCV_INCLUDE_DIRS})
target_compile_options(superviseur-pthreads PUBLIC ${OPENCV_CFLAGS_OTHER})

#target_link_libraries(superviseur-pthreads PRIVATE Threads::Threads)
#target_include_directories(superviseur-pthreads PUBLIC ${PTHREAD_INCLUDE_DIRS})
#target_compile_options(superviseur-pthreads PUBLIC ${PTHREAD_CFLAGS_OTHER})