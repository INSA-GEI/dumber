/* 
 * File:   Program.c
 * Author: dimercur
 *
 * Created on 23 octobre 2018, 19:45
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include <sys/mman.h>

#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <sched.h>

#include "Tasks.h"

using namespace std;

class Program {
public:
// Déclaration des handleurs de tache
    pthread_t th_server;
    pthread_t th_sendToMon;
    pthread_t th_receiveFromMon;
    pthread_t th_openComRobot;
    pthread_t th_startRobot;
    pthread_t th_move;

// Déclaration des priorités des taches
    int PRIORITY_TSERVER = 30;
    int PRIORITY_TOPENCOMROBOT = 20;
    int PRIORITY_TMOVE = 10;
    int PRIORITY_TSENDTOMON = 25;
    int PRIORITY_TRECEIVEFROMMON = 22;
    int PRIORITY_TSTARTROBOT = 20;

// Declaration de mutex
    pthread_mutex_t mutex_robotStarted;
    pthread_mutex_t mutex_move;

//declaration de semaphores
    sem_t sem_barrier;
    sem_t sem_openComRobot;
    sem_t sem_serverOk;
    sem_t sem_startRobot;

// Déclaration des files de message
    mqd_t q_messageToMon;
    struct mq_attr q_messageToMonAttr;

    int MSG_QUEUE_SIZE = 10;

// Déclaration des ressources partagées
    int etatCommMoniteur = 1;
    int robotStarted = 0;

    Program();
    virtual ~Program();

/**
 * \fn void initStruct(void)
 * \brief Initialisation des structures de l'application (tâches, mutex, 
 * semaphore, etc.)
 */
    void initStruct(void);

/**
 * \fn void startTasks(void)
 * \brief Démarrage des tâches
 */
    void startTasks(void);

/**
 * \fn void deleteTasks(void)
 * \brief Arrêt des tâches
 */
    void deleteTasks(void);

    /**
     * \fn int run(void)
     * \brief execute program
     */
    int run(void);
};

int main(int argc, char **argv) {
    int err;
    Program myProg;
    TcpServer server;

    Tasks::messages.SetServer(&server);

    //Lock the memory to avoid memory swapping for this program
    mlockall(MCL_CURRENT | MCL_FUTURE);

    printf("#################################\n");
    printf("#      DE STIJL PROJECT         #\n");
    printf("#################################\n");

    return myProg.run();
}

Program::Program() {}
Program::~Program() {}

int Program::run(void) {

    this->initStruct();
    this->startTasks();
    //sem_post(&sem_barrier);
    pause();
    this->deleteTasks();
}

void Program::initStruct(void) {

    int err;

    /* Creation des mutex */
    mutex_robotStarted=PTHREAD_MUTEX_INITIALIZER;
    mutex_move=PTHREAD_MUTEX_INITIALIZER;

    /* Creation du semaphore */
    if (sem_init(&sem_barrier, 0, 0) == -1) {
        printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (sem_init(&sem_openComRobot, 0, 0) == -1) {
        printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (sem_init(&sem_serverOk, 0, 0) == -1) {
        printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    if (sem_init(&sem_startRobot, 0, 0) == -1) {
        printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* Sous pthread, les taches sont executées dés leur creation
     * la creation des taches se fait donc dans startTask */

    /* Creation des files de messages */
    q_messageToMonAttr.mq_flags = 0;
    q_messageToMonAttr.mq_maxmsg = 10;
    q_messageToMonAttr.mq_msgsize = 200;
    q_messageToMonAttr.mq_curmsgs = 0;

    q_messageToMon = mq_open("MessageToMon", O_CREAT | O_RDWR, 0644, &q_messageToMonAttr);
    if((mqd_t)-1 != q_messageToMon){
        printf("Error msg queue create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

//    if (err = rt_queue_create(&q_messageToMon, "toto", MSG_QUEUE_SIZE * sizeof (MessageToRobot), MSG_QUEUE_SIZE, Q_FIFO)) {
//        printf("Error msg queue create: %s\n", strerror(-err));
//        exit(EXIT_FAILURE);
//    }
}

void Program::startTasks() {

    int err;
    pthread_attr_t tattr;
    pthread_t tid;
    int ret;
    sched_param param;

    /* initialized with default attributes */
    ret = pthread_attr_init (&tattr);

    /* safe to get existing scheduling param */
    ret = pthread_attr_getschedparam (&tattr, &param);

    /* set the priority; others are unchanged */
    param.sched_priority = PRIORITY_TSTARTROBOT;

    /* setting the new scheduling param */
    ret = pthread_attr_setschedparam (&tattr, &param);

    /* Creation des taches */

    if ((err = pthread_create(&th_startRobot, &tattr, (void* (*)(void*))Tasks::f_startRobot, NULL))) {
        printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* safe to get existing scheduling param */
    ret = pthread_attr_getschedparam (&tattr, &param);

    /* set the priority; others are unchanged */
    param.sched_priority = PRIORITY_TRECEIVEFROMMON;

    /* setting the new scheduling param */
    ret = pthread_attr_setschedparam (&tattr, &param);

    if ((err = pthread_create(&th_receiveFromMon, &tattr, (void* (*)(void*))Tasks::f_receiveFromMon, NULL))) {
        printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* safe to get existing scheduling param */
    ret = pthread_attr_getschedparam (&tattr, &param);

    /* set the priority; others are unchanged */
    param.sched_priority = PRIORITY_TSENDTOMON;

    /* setting the new scheduling param */
    ret = pthread_attr_setschedparam (&tattr, &param);

    if ((err = pthread_create(&th_sendToMon, &tattr, (void* (*)(void*))Tasks::f_sendToMon, NULL))) {
        printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* safe to get existing scheduling param */
    ret = pthread_attr_getschedparam (&tattr, &param);

    /* set the priority; others are unchanged */
    param.sched_priority = PRIORITY_TOPENCOMROBOT;

    /* setting the new scheduling param */
    ret = pthread_attr_setschedparam (&tattr, &param);

    if ((err = pthread_create(&th_openComRobot, &tattr, (void* (*)(void*))Tasks::f_openComRobot, NULL))) {
        printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* safe to get existing scheduling param */
    ret = pthread_attr_getschedparam (&tattr, &param);

    /* set the priority; others are unchanged */
    param.sched_priority = PRIORITY_TMOVE;

    /* setting the new scheduling param */
    ret = pthread_attr_setschedparam (&tattr, &param);

    if ((err = pthread_create(&th_move, &tattr, (void* (*)(void*))Tasks::f_move, NULL)))  {
        printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* safe to get existing scheduling param */
    ret = pthread_attr_getschedparam (&tattr, &param);

    /* set the priority; others are unchanged */
    param.sched_priority = PRIORITY_TSERVER;

    /* setting the new scheduling param */
    ret = pthread_attr_setschedparam (&tattr, &param);

    if ((err = pthread_create(&th_server, &tattr, (void* (*)(void*))Tasks::f_server, NULL))) {
        printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
}

void Program::deleteTasks() {
    pthread_cancel(th_server);
    pthread_cancel(th_openComRobot);
    pthread_cancel(th_move);
}
