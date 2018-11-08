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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/e4d40e25/image.o \
	${OBJECTDIR}/_ext/e4d40e25/message.o \
	${OBJECTDIR}/_ext/e4d40e25/robot.o \
	${OBJECTDIR}/_ext/e4d40e25/server.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs opencv` -lpthread   

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testeur

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testeur: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/testeur ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/e4d40e25/image.o: ../../superviseur-robot/lib/src/image.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e4d40e25
	${RM} "$@.d"
	$(COMPILE.cc) -g -D__FOR_PC__ -DD_REENTRANT -I../../superviseur-robot/lib `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e4d40e25/image.o ../../superviseur-robot/lib/src/image.cpp

${OBJECTDIR}/_ext/e4d40e25/message.o: ../../superviseur-robot/lib/src/message.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e4d40e25
	${RM} "$@.d"
	$(COMPILE.cc) -g -D__FOR_PC__ -DD_REENTRANT -I../../superviseur-robot/lib `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e4d40e25/message.o ../../superviseur-robot/lib/src/message.cpp

${OBJECTDIR}/_ext/e4d40e25/robot.o: ../../superviseur-robot/lib/src/robot.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e4d40e25
	${RM} "$@.d"
	$(COMPILE.cc) -g -D__FOR_PC__ -DD_REENTRANT -I../../superviseur-robot/lib `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e4d40e25/robot.o ../../superviseur-robot/lib/src/robot.cpp

${OBJECTDIR}/_ext/e4d40e25/server.o: ../../superviseur-robot/lib/src/server.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e4d40e25
	${RM} "$@.d"
	$(COMPILE.cc) -g -D__FOR_PC__ -DD_REENTRANT -I../../superviseur-robot/lib `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e4d40e25/server.o ../../superviseur-robot/lib/src/server.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D__FOR_PC__ -DD_REENTRANT -I../../superviseur-robot/lib `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

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
