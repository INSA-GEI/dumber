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
 * \file      message.cpp
 * \author    PE.Hladik
 * \version   1.0
 * \date      06/06/2017
 * \brief     Functions for sending message to monitor.
 */

#include "message.h"

void set_msgToMon_header(MessageToMon *msg, char *header) {
    memcpy(msg->header, header, sizeof (header));
}

void set_msgToMon_data(MessageToMon *msg, void * data) {
    if (msg->data != NULL) {
        free(msg->data);
    }
    msg->data = malloc(sizeof (data));
    memcpy(msg->data, data, sizeof (data));
}

void free_msgToMon_data(MessageToMon *msg) {
    if (msg->data != NULL) {
        free(msg->data);
    }
}

void print_msgToMon(MessageToMon *msg) {
    printf("header: %s\n", msg->header);
    printf("data: %s\n", (char*)msg->data);
}
