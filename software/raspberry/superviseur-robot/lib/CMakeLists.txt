set(LIB_FILES ./src/Serial.cpp ./src/Robot.cpp ./src/TcpServer.cpp)
include_directories("./")

add_library(destjil STATIC ${LIB_FILES})

