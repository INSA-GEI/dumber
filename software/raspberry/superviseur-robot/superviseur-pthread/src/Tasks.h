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

#ifndef TASKS_H
#define TASKS_H

#include <mqueue.h>
#include <MsgManager.h>

class Tasks {
public:
    static MsgManager messages;

    static void f_server(void *arg);

    static void f_sendToMon(void *arg);

    static void f_receiveFromMon(void *arg);

    static void f_openComRobot(void *arg);

    static void f_move(void *arg);

    static void f_startRobot(void *arg);

private:
    char mode_start;

    static void write_in_queue(mqd_t *queue, MsgManager msg);
    Tasks() {}
};

#endif /* TASKS_H */

