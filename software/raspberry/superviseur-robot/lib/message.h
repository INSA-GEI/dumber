/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   message.h
 * Author: pehladik
 *
 * Created on 7 février 2018, 17:20
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char header[4];
    void *data = NULL;
} MessageToMon;

void set_msgToMon_header(MessageToMon *msg, char *header);
void set_msgToMon_data(MessageToMon *msg, void * data);
void free_msgToMon_data(MessageToMon *msg);
void print_msgToMon(MessageToMon *msg);

#endif /* MESSAGE_H */

