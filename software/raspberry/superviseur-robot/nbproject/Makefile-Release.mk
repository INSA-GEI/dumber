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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include ./Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/lib/image.o \
	${OBJECTDIR}/lib/message.o \
	${OBJECTDIR}/lib/messages.o \
	${OBJECTDIR}/lib/monitor.o \
	${OBJECTDIR}/lib/robot.o \
	${OBJECTDIR}/lib/server.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/tasks.o \
	${OBJECTDIR}/_ext/6cc0dc4a/camera.o \
	${OBJECTDIR}/_ext/6cc0dc4a/commonitor.o \
	${OBJECTDIR}/_ext/6cc0dc4a/comrobot.o \
	${OBJECTDIR}/_ext/6cc0dc4a/img.o \
	${OBJECTDIR}/tasks_pthread.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur-robot

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur-robot: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/superviseur-robot ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/lib/image.o: lib/image.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/image.o lib/image.cpp

${OBJECTDIR}/lib/message.o: lib/message.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/message.o lib/message.cpp

${OBJECTDIR}/lib/messages.o: lib/messages.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/messages.o lib/messages.cpp

${OBJECTDIR}/lib/monitor.o: lib/monitor.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/monitor.o lib/monitor.cpp

${OBJECTDIR}/lib/robot.o: lib/robot.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/robot.o lib/robot.cpp

${OBJECTDIR}/lib/server.o: lib/server.cpp
	${MKDIR} -p ${OBJECTDIR}/lib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib/server.o lib/server.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/tasks.o: tasks.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tasks.o tasks.cpp

${OBJECTDIR}/_ext/6cc0dc4a/camera.o: /home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/lib/camera.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6cc0dc4a
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6cc0dc4a/camera.o /home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/lib/camera.cpp

${OBJECTDIR}/_ext/6cc0dc4a/commonitor.o: /home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/lib/commonitor.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6cc0dc4a
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6cc0dc4a/commonitor.o /home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/lib/commonitor.cpp

${OBJECTDIR}/_ext/6cc0dc4a/comrobot.o: /home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/lib/comrobot.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6cc0dc4a
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6cc0dc4a/comrobot.o /home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/lib/comrobot.cpp

${OBJECTDIR}/_ext/6cc0dc4a/img.o: /home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/lib/img.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6cc0dc4a
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6cc0dc4a/img.o /home/dimercur/Documents/Travail/git/dumber/software/raspberry/superviseur-robot/lib/img.cpp

${OBJECTDIR}/tasks_pthread.o: tasks_pthread.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tasks_pthread.o tasks_pthread.cpp

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
