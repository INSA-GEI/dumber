################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../lib/Robot.cpp \
../lib/Serial.cpp \
../lib/TcpServer.cpp \
../lib/message.cpp 

OBJS += \
./lib/Robot.o \
./lib/Serial.o \
./lib/TcpServer.o \
./lib/message.o 

CPP_DEPS += \
./lib/Robot.d \
./lib/Serial.d \
./lib/TcpServer.d \
./lib/message.d 


# Each subdirectory must supply rules for building sources it contributes
lib/%.o: ../lib/%.cpp
	g++ -I"/home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/example/lib" -I"/home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/example/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"

