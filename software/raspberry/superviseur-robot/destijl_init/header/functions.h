/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   functions.h
 * Author: pehladik
 *
 * Created on 15 janvier 2018, 12:50
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/mutex.h>
#include <alchemy/sem.h>
#include <alchemy/queue.h>

#include "../../src/monitor.h"    
#include "../../src/robot.h"
#include "../../src/image.h"
#include "../../src/message.h"

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
extern char move;

extern int MSG_QUEUE_SIZE;

extern int PRIORITY_TSERVER;
extern int PRIORITY_TOPENCOMROBOT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TSENDTOMON;
extern int PRIORITY_TRECEIVEFROMMON;
extern int PRIORITY_TSTARTROBOT;

void f_server(void *arg);
void f_sendToMon(void *arg);
void f_receiveFromMon(void *arg);
void f_openComRobot(void * arg);
void f_move(void *arg);
void f_startRobot(void *arg);

#endif /* FUNCTIONS_H */

