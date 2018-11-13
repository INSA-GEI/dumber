set(superviseur_FILES ./src/main.cpp src/functions.cpp)

include_directories(./src ../lib /usr/xenomai/include /usr/xenomai/include/mercury)
link_directories(/usr/xenomai/lib)

add_executable(superviseur ${superviseur_FILES})
target_link_libraries(superviseur destijl)

#add_custom_command(TARGET superviseur POST_BUILD
#        COMMAND sudo chown root ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/superviseur
#        COMMAND sudo chgrp root ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/superviseur
#        COMMAND sudo chmod +s ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/superviseur
#        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#        COMMENT "Add sticky bit"
#)