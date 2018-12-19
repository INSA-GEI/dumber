/*
 * Copyright (C) 2018 dimercur
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file      functions.h
 * \author    PE.Hladik
 * \version   1.0
 * \date      06/06/2017
 * \brief     Miscellaneous functions used for destijl project.
 */

#ifndef __TASKS_H__
#define __TASKS_H__

#ifdef __WITH_PTHREAD__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>

#include "monitor.h"
#include "robot.h"
#include "image.h"
#include "message.h"
#include "server.h"

extern RT_TASK th_server;
extern RT_TASK th_sendToMon;
extern RT_TASK th_receiveFromMon;
extern RT_TASK th_openComRobot;
extern RT_TASK th_startRobot;
extern RT_TASK th_move;

extern RT_MUTEX mutex_robotStarted;
extern RT_MUTEX mutex_move;

extern RT_SEM sem_barrier;
extern RT_SEM sem_openComRobot;
extern RT_SEM sem_serverOk;
extern RT_SEM sem_startRobot;

extern RT_QUEUE q_messageToMon;

extern int etatCommMoniteur;
extern int robotStarted;
extern char robotMove;

extern int MSG_QUEUE_SIZE;

extern int PRIORITY_TSERVER;
extern int PRIORITY_TOPENCOMROBOT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TSENDTOMON;
extern int PRIORITY_TRECEIVEFROMMON;
extern int PRIORITY_TSTARTROBOT;

/**
 * \brief       Thread handling server communication.
 */ 
void f_server(void *arg);

/**
 * \brief       Thread handling communication to monitor.
 */ 
void f_sendToMon(void *arg);

/**
 * \brief       Thread handling communication from monitor.
 */ 
void f_receiveFromMon(void *arg);

/**
 * \brief       Thread handling opening of robot communication.
 */ 
void f_openComRobot(void * arg);

/**
 * \brief       Thread handling robot mouvements.
 */ 
void f_move(void *arg);

/**
 * \brief       Thread handling robot activation.
 */ 
void f_startRobot(void *arg);

#endif // __WITH_PTHREAD__
#endif /* __TASKS_H__ */

