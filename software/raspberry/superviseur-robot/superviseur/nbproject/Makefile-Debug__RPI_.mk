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
CND_CONF=Debug__RPI_
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/a59f760b/image.o \
	${OBJECTDIR}/_ext/a59f760b/message.o \
	${OBJECTDIR}/_ext/a59f760b/monitor.o \
	${OBJECTDIR}/_ext/a59f760b/robot.o \
	${OBJECTDIR}/_ext/a59f760b/server.o \
	${OBJECTDIR}/src/functions.o \
	${OBJECTDIR}/src/main.o


# C Compiler Flags
CFLAGS=-I/usr/xenomai/include/mercury -I/usr/xenomai/include -D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -D__MERCURY__ -I/usr/xenomai/include/alchemy

# CC Compiler Flags
CCFLAGS=-I/usr/xenomai/include/mercury -I/usr/xenomai/include -D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -D__MERCURY__ -I/usr/xenomai/include/alchemy
CXXFLAGS=-I/usr/xenomai/include/mercury -I/usr/xenomai/include -D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -D__MERCURY__ -I/usr/xenomai/include/alchemy

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs opencv` -lraspicam_cv -lraspicam  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--no-as-needed -lalchemy -lcopperplate /usr/xenomai/lib/xenomai/bootstrap.o -Wl,--wrap=main -Wl,--dynamic-list=/usr/xenomai/lib/dynlist.ld -L/usr/xenomai/lib -lmercury -lpthread -lrt

${OBJECTDIR}/_ext/a59f760b/image.o: ../lib/src/image.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f760b
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -Isrc -I../lib -I/usr/xenomai/include -I/usr/xenomai/include/mercury `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f760b/image.o ../lib/src/image.cpp

${OBJECTDIR}/_ext/a59f760b/message.o: ../lib/src/message.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f760b
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -Isrc -I../lib -I/usr/xenomai/include -I/usr/xenomai/include/mercury `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f760b/message.o ../lib/src/message.cpp

${OBJECTDIR}/_ext/a59f760b/monitor.o: ../lib/src/monitor.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f760b
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -Isrc -I../lib -I/usr/xenomai/include -I/usr/xenomai/include/mercury `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f760b/monitor.o ../lib/src/monitor.cpp

${OBJECTDIR}/_ext/a59f760b/robot.o: ../lib/src/robot.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f760b
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -Isrc -I../lib -I/usr/xenomai/include -I/usr/xenomai/include/mercury `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f760b/robot.o ../lib/src/robot.cpp

${OBJECTDIR}/_ext/a59f760b/server.o: ../lib/src/server.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f760b
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -Isrc -I../lib -I/usr/xenomai/include -I/usr/xenomai/include/mercury `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f760b/server.o ../lib/src/server.cpp

${OBJECTDIR}/src/functions.o: src/functions.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -Isrc -I../lib -I/usr/xenomai/include -I/usr/xenomai/include/mercury `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/functions.o src/functions.cpp

${OBJECTDIR}/src/main.o: src/main.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_WITH_TRACE_ -Isrc -I../lib -I/usr/xenomai/include -I/usr/xenomai/include/mercury `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.cpp

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
