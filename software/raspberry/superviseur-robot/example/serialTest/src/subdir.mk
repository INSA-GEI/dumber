################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/serialTest.cpp \
../src/serverTest.cpp 

OBJS += \
./src/serialTest.o \
./src/serverTest.o 

CPP_DEPS += \
./src/serialTest.d \
./src/serverTest.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	g++ -I"/home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/example/lib" -I"/home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/example/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"

