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

#include "tasks.h"
#include <stdexcept>

// Déclaration des priorités des taches
#define PRIORITY_TSERVER 30
#define PRIORITY_TOPENCOMROBOT 20
#define PRIORITY_TMOVE 10
#define PRIORITY_TSENDTOMON 22
#define PRIORITY_TRECEIVEFROMMON 25
#define PRIORITY_TSTARTROBOT 20
#define PRIORITY_TCAMERA 21

/*
 * Some remarks:
 * 1- This program is mostly a template. It shows you how to create tasks, semaphore
 *   message queues, mutex ... and how to use them
 * 
 * 2- semDumber is, as name say, useless. Its goal is only to show you how to use semaphore
 * 
 * 3- Data flow is probably not optimal
 * 
 * 4- Take into account that ComRobot::Write will block your task when serial buffer is full,
 *   time for internal buffer to flush
 * 
 * 5- Same behavior existe for ComMonitor::Write !
 * 
 * 6- When you want to write something in terminal, use cout and terminate with endl and flush
 * 
 * 7- Good luck !
 */

/**
 * @brief Initialisation des structures de l'application (tâches, mutex, 
 * semaphore, etc.)
 */
void Tasks::Init() {
    int status;
    int err;

    /**************************************************************************************/
    /* 	Mutex creation                                                                    */
    /**************************************************************************************/
    if (err = rt_mutex_create(&mutex_robotStarted, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_move, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Mutexes created successfully" << endl << flush;

    /**************************************************************************************/
    /* 	Semaphors creation       							  */
    /**************************************************************************************/
    if (err = rt_sem_create(&sem_barrier, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_openComRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_serverOk, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_startRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Semaphores created successfully" << endl << flush;

    /**************************************************************************************/
    /* Tasks creation                                                                     */
    /**************************************************************************************/
    if (err = rt_task_create(&th_server, "th_server", 0, PRIORITY_TSERVER, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_receiveFromMon, "th_receiveFromMon", 0, PRIORITY_TRECEIVEFROMMON, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_sendToMon, "th_sendToMon", 0, PRIORITY_TSENDTOMON, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_openComRobot, "th_openComRobot", 0, PRIORITY_TOPENCOMROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_startRobot, "th_startRobot", 0, PRIORITY_TSTARTROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_move, "th_move", 0, PRIORITY_TMOVE, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_camera, "th_camera", 0, PRIORITY_TCAMERA, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Tasks created successfully" << endl << flush;

    /**************************************************************************************/
    /* Message queues creation                                                            */
    /**************************************************************************************/
    if ((err = rt_queue_create(&q_messageToMon, "q_messageToMon", sizeof (Message*)*50, Q_UNLIMITED, Q_FIFO)) < 0) {
        cerr << "Error msg queue create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Queues created successfully" << endl << flush;

    /* Open com port with STM32 */
    cout << "Open serial com (";
    status = robot.Open();
    cout << status;
    cout << ")" << endl;

    if (status >= 0) {
        // Open server

        status = monitor.Open(SERVER_PORT);
        cout << "Open server on port " << (SERVER_PORT) << " (" << status << ")" << endl;

        if (status < 0) throw std::runtime_error {
            "Unable to start server on port " + std::to_string(SERVER_PORT)
        };
    } else
        throw std::runtime_error {
        "Unable to open serial port /dev/ttyS0 "
    };
}

/**
 * @brief Démarrage des tâches
 */
void Tasks::Run() {
    int err;

    if (err = rt_task_start(&th_receiveFromMon, (void(*)(void*)) & Tasks::ReceiveFromMonTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }

    if (err = rt_task_start(&th_camera, (void(*)(void*)) & Tasks::CameraTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }

    //    if (err = rt_task_start(&th_sendToMon, (void(*)(void*)) & Tasks::SendToMonTask, this)) {
    //        cerr << "Error task start: " << strerror(-err) << endl << flush;
    //        exit(EXIT_FAILURE);
    //    }

    cout << "Tasks launched" << endl << flush;
}

/**
 * @brief Arrêt des tâches
 */
void Tasks::Stop() {
    monitor.Close();
    robot.Close();
}

/**
 */
void Tasks::Join() {
    rt_sem_broadcast(&sem_barrier);
    pause();
}

/**
 * @brief Thread handling server communication.
 */

void Tasks::ReceiveFromMonTask(void *arg) {
    Message *msgRcv;
    Message *msgSend;
    bool isActive = true;

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    while (isActive) {
        msgRcv = NULL;
        msgSend = NULL;

        msgRcv = monitor.Read();
        cout << "Rcv <= " << msgRcv->ToString() << endl << flush;

        if (msgRcv->CompareID(MESSAGE_ROBOT_COM_OPEN)) {
            msgSend = new Message(MESSAGE_ANSWER_ACK);
            isActive = true;

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_ROBOT_COM_CLOSE)) {
            msgSend = new Message(MESSAGE_ANSWER_ACK);
            cout << "isActive = false!" << msgRcv->ToString() << endl << flush;
            isActive = false;

            delete(msgRcv); // mus be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITH_WD)) {
            msgSend = robot.Write(msgRcv);
            cout << "Start with wd answer: " << msgSend->ToString() << endl << flush;
        }

        if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITHOUT_WD)) {
            msgSend = robot.Write(msgRcv);
            cout << "Start without wd answer: " << msgSend->ToString() << endl << flush;
        }

        if (msgRcv->CompareID(MESSAGE_ROBOT_RESET)) {
            msgSend = robot.Write(msgRcv);
            cout << "Reset answer: " << msgSend->ToString() << endl << flush;
        }

        if (msgRcv->CompareID(MESSAGE_ROBOT_GO_FORWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_BACKWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_LEFT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_RIGHT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_STOP)) {
            msgSend = robot.Write(msgRcv);

            cout << "Movement answer: " << msgSend->ToString() << endl << flush;

            if (msgSend->CompareID(MESSAGE_ANSWER_ACK)) {
                delete (msgSend);
                msgSend = NULL;
            }
        }

        if (msgRcv->CompareID(MESSAGE_CAM_OPEN)) {
            sendImage = true;
            msgSend = new Message(MESSAGE_ANSWER_ACK);

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_CAM_CLOSE)) {
            sendImage = false;
            msgSend = new Message(MESSAGE_ANSWER_ACK);

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_CAM_POSITION_COMPUTE_START)) {
            sendPosition = true;
            msgSend = new Message(MESSAGE_ANSWER_ACK);

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_CAM_POSITION_COMPUTE_STOP)) {
            sendPosition = false;
            msgSend = new Message(MESSAGE_ANSWER_ACK);

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_ROBOT_BATTERY_GET)) {
            msgSend = new MessageBattery(MESSAGE_ROBOT_BATTERY_LEVEL, BATTERY_FULL);

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_CAM_ASK_ARENA)) {
            showArena = true;

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_CAM_ARENA_CONFIRM)) {
            showArena = false;

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgRcv->CompareID(MESSAGE_CAM_ARENA_INFIRM)) {
            showArena = false;

            delete(msgRcv); // must be deleted manually, no consumer
        }

        if (msgSend != NULL) monitor.Write(msgSend);
    }
}

/**
 * @brief Thread handling periodic image capture.
 */
void Tasks::CameraTask(void* arg) {
    struct timespec tim, tim2;
    Message *msgSend;
    int counter;
    int cntFrame = 0;
    Position pos;
    Arena arena;

    tim.tv_sec = 0;
    tim.tv_nsec = 50000000; // 50ms (20fps)

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    Camera camera = Camera(sm, 20);
    cout << "Try opening camera" << endl << flush;
    if (camera.Open()) cout << "Camera opened successfully" << endl << flush;
    else {
        cout << "Failed to open camera" << endl << flush;

        exit(0);
    }

    while (1) {
        
    }
}

/**
 * @brief Thread sending data to monitor.
 */
void Tasks::SendToMonTask(void* arg) {
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    while (1) {

    }
}

/**
 * Write a message in a given queue
 * @param queue Queue identifier
 * @param msg Message to be stored
 */
void WriteInQueue(RT_QUEUE &queue, Message *msg) {
    int err;

    if ((err = rt_queue_send(&queue, (const void *) msg, sizeof ((const void *) msg), Q_NORMAL)) < 0) {
        cerr << "Write in queue failed: " << strerror(-err) << endl << flush;
        throw std::runtime_error{"Error in write in queue"};
    }
}

/**
 * Read a message from a given queue, block if empty
 * @param queue Queue identifier
 * @return Message read
 */
Message *ReadInQueue(RT_QUEUE &queue) {
    int err;
    Message *msg;

    if ((err = rt_queue_read(&queue, (void*) msg, sizeof ((void*) msg), TM_INFINITE)) < 0) {
        cerr << "Write in queue failed: " << strerror(-err) << endl << flush;
        throw std::runtime_error{"Error in write in queue"};
    }

    return msg;
}

