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

#ifdef __WITH_PTHREAD__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "camera.h"
#include "img.h"

#include "messages.h"
#include "commonitor.h"
#include "comrobot.h"

#include <thread>
#include <mutex>
#include <condition_variable>

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
        threadServer->join();
        threadTimer->join();
        threadSendToMon->join();
    }
    
    /**
     */
    bool AcceptClient() {
        return monitor.AcceptClient();
    }
    
     /**
     * @brief Thread handling server communication.
     */
    void ServerTask(void *arg);

    /**
     * @brief Thread handling server communication.
     */
    void TimerTask(void *arg);
    
    /**
     * @brief Thread handling communication to monitor.
     */
    void SendToMonTask(void *arg);
private:
    ComMonitor monitor;
    ComRobot robot;
    
    bool sendImage=false;
    bool sendPosition=false;
    
    int counter;
    bool flag;
    
    bool showArena=false;
    
    thread *threadServer;
    thread *threadSendToMon;
    thread *threadTimer;
    
    mutex mutexTimer;
};

#endif // __WITH_PTHREAD__
#endif /* __TASKS_H__ */

