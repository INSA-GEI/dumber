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

#include <unistd.h>
#include <iostream>

#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/mutex.h>
#include <alchemy/sem.h>
#include <alchemy/queue.h>

#include "messages.h"
#include "commonitor.h"
#include "comrobot.h"
#include "camera.h"
#include "img.h"

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
    void Join();
    
    /**
     */
    bool AcceptClient() {
        return monitor.AcceptClient();
    }
    
private:
    ComMonitor monitor;
    ComRobot robot;
    
    bool sendImage=false;
    bool sendPosition=false;
    
    int counter;
    bool flag;
    
    bool showArena=false;
    
    RT_TASK th_server;
    RT_TASK th_sendToMon;
    RT_TASK th_receiveFromMon;
    RT_TASK th_openComRobot;
    RT_TASK th_startRobot;
    RT_TASK th_move;
    RT_TASK th_camera;

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

    char mode_start;
    
    /**
     * Write a message in a given queue
     * @param queue Queue identifier
     * @param msg Message to be stored
     */
    void WriteInQueue(RT_QUEUE *queue, Message *msg);

    /**
     * Read a message from a given queue, block if empty
     * @param queue Queue identifier
     * @return Message read
     */
    Message *ReadInQueue(RT_QUEUE *queue);
    
    /**
     * @brief Thread handling server communication.
     */
    void ReceiveFromMonTask(void *arg);

    /**
     * @brief Thread handling periodic image capture.
     */
    void CameraTask(void *arg);
    
    /**
     * @brief Thread sending data to monitor.
     */
    void SendToMonTask(void *arg);
//    /**
//     * \brief       Thread handling server communication.
//     */
//    void f_server(void *arg);
//
//    /**
//     * \brief       Thread handling communication to monitor.
//     */
//    void f_sendToMon(void *arg);
//
//    /**
//     * \brief       Thread handling communication from monitor.
//     */
//    void f_receiveFromMon(void *arg);
//
//    /**
//     * \brief       Thread handling opening of robot communication.
//     */
//    void f_openComRobot(void * arg);
//
//    /**
//     * \brief       Thread handling robot mouvements.
//     */
//    void f_move(void *arg);
//
//    /**
//     * \brief       Thread handling robot activation.
//     */
//    void f_startRobot(void *arg);
};

#endif // __TASKS_H__ 

