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
 * \file      message.h
 * \author    PE.Hladik
 * \version   1.0
 * \date      06/06/2017
 * \brief     Functions for sending message to monitor.
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

/**
 * \brief       Set header part of monitor message.
 *             
 */ 
void set_msgToMon_header(MessageToMon *msg, char *header);

/**
 * \brief       Set data part of monitor message.
 */ 
void set_msgToMon_data(MessageToMon *msg, void *data);

/**
 * \brief       Dealocate moemory pointed by data part of message.
 */ 
void free_msgToMon_data(MessageToMon *msg);

/**
 * \brief       Print message, header and data.
 */ 
void print_msgToMon(MessageToMon *msg);

#endif /* MESSAGE_H */

