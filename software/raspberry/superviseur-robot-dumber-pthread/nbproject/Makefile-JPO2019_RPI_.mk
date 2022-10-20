#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=JPO2019_RPI_
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include ./Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/lib/base64/base64.o \
	${OBJECTDIR}/lib/camera.o \
	${OBJECTDIR}/lib/commonitor.o \
	${OBJECTDIR}/lib/comrobot.o \
	${OBJECTDIR}/lib/img.o \
	${OBJECTDIR}/lib/messages.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/tasks_pthread.o


# C Compiler Flags
CFLAGS=-I/usr/xenomai/include/mercury -I/usr/xenomai/include -D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -D__MERCURY__ -I/usr/xenomai/include/alchemy

# CC Compiler Flags
CCFLAGS=-D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -Wno-pmf-conversions -std=gnu++11
CXXFLAGS=-D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -Wno-pmf-conversions -std=gnu++11

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs opencv4`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur-robot-dumber-pthread

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur-robot-dumber-pthread: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur-robot-dumber-pthread ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--no-as-needed -lpthread -lrt

${OBJECTDIR}/lib/base64/base64.o: lib/base64/base64.cpp
	${MKDIR} -p ${OBJECTDIR}/lib/base64
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -D__WITH_ARUCO__ -D__WITH_PTHREAD__ -I./ -I./lib `pkg-config --cflags opencv4` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/base64/base64.o lib/base64/base64.cpp

${OBJECTDIR}/lib/camera.o: lib/camera.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -D__WITH_ARUCO__ -D__WITH_PTHREAD__ -I./ -I./lib `pkg-config --cflags opencv4` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/camera.o lib/camera.cpp

${OBJECTDIR}/lib/commonitor.o: lib/commonitor.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -D__WITH_ARUCO__ -D__WITH_PTHREAD__ -I./ -I./lib `pkg-config --cflags opencv4` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/commonitor.o lib/commonitor.cpp

${OBJECTDIR}/lib/comrobot.o: lib/comrobot.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -D__WITH_ARUCO__ -D__WITH_PTHREAD__ -I./ -I./lib `pkg-config --cflags opencv4` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/comrobot.o lib/comrobot.cpp

${OBJECTDIR}/lib/img.o: lib/img.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -D__WITH_ARUCO__ -D__WITH_PTHREAD__ -I./ -I./lib `pkg-config --cflags opencv4` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/img.o lib/img.cpp

${OBJECTDIR}/lib/messages.o: lib/messages.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -D__WITH_ARUCO__ -D__WITH_PTHREAD__ -I./ -I./lib `pkg-config --cflags opencv4` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/messages.o lib/messages.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -D__WITH_ARUCO__ -D__WITH_PTHREAD__ -I./ -I./lib `pkg-config --cflags opencv4` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/tasks_pthread.o: tasks_pthread.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -D__WITH_ARUCO__ -D__WITH_PTHREAD__ -I./ -I./lib `pkg-config --cflags opencv4` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tasks_pthread.o tasks_pthread.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
