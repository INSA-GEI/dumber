set(LIB_FILES ./src/robot.cpp ./src/monitor.cpp ./src/message.cpp ./src/image.cpp)
include_directories("./")

add_library(destijl STATIC ${LIB_FILES})

