/*
 * Copyright (C) 2024 dimercur
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

#include <pthread.h>

class Tasks
{
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
    void Join()
    {
        void *ret;

        if (pthread_join(threadServer, &ret) != 0)
        {
            perror("pthread_create() error");
            exit(3);
        }

        if (pthread_join(threadTimer, &ret) != 0)
        {
            perror("pthread_create() error");
            exit(3);
        }

        if (pthread_join(threadSendToMon, &ret) != 0)
        {
            perror("pthread_create() error");
            exit(3);
        }
    }

    /**
     */
    bool AcceptClient()
    {
        return monitor.AcceptClient();
    }

    /**
     * @brief Thread handling server communication.
     */
    void *ServerTask(void *arg);

    /**
     * @brief Thread handling server communication.
     */
    void *TimerTask(void *arg);

    /**
     * @brief Thread handling communication to monitor.
     */
    void SendToMonTask(void *arg);

    static ComMonitor monitor;
    static ComRobot robot;

    static bool sendImage;
    static bool sendPosition;

    static int counter;
    static bool flag;

    static bool showArena;

    static pthread_t threadServer;
    static pthread_t threadSendToMon;
    static pthread_t threadTimer;
};

#endif // __WITH_PTHREAD__
#endif /* __TASKS_H__ */
