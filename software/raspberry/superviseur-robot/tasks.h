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

#ifndef __TASKS_H__
#define __TASKS_H__

#ifndef __WITH_PTHREAD__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/mutex.h>
#include <alchemy/sem.h>
#include <alchemy/queue.h>

//#include "monitor.h"
//#include "robot.h"
//#include "image.h"
//#include "message.h"
//#include "server.h"

#include "messages.h"
#include "commonitor.h"
#include "comrobot.h"

using namespace std;

class Tasks {
public:
    /**
     * @brief Initialisation des structures de l'application (tâches, mutex, 
     * semaphore, etc.)
     */
    void Init();

    /**
     * @brief Démarrage des tâches
     */
    void Run();

    /**
     * @brief Arrêt des tâches
     */
    void Stop();
    
    /**
     */
    void Join() {
        rt_sem_broadcast(&sem_barrier);
        pause();
    }
    
    /**
     */
    bool AcceptClient() {
        return false;
    }
    
private:
    ComMonitor monitor;
    ComRobot robot;
    
    RT_TASK th_server;
    RT_TASK th_sendToMon;
    RT_TASK th_receiveFromMon;
    RT_TASK th_openComRobot;
    RT_TASK th_startRobot;
    RT_TASK th_move;

    RT_MUTEX mutex_robotStarted;
    RT_MUTEX mutex_move;

    RT_SEM sem_barrier;
    RT_SEM sem_openComRobot;
    RT_SEM sem_serverOk;
    RT_SEM sem_startRobot;

    RT_QUEUE q_messageToMon;

    int etatCommMoniteur;
    int robotStarted;
    char robotMove;

    int MSG_QUEUE_SIZE;

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
};

#endif // __WITH_PTHREAD__
#endif // __TASKS_H__ 

