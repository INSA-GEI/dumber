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
 * \file      server.cpp
 * \author    PE.Hladik
 * \version   1.0
 * \date      06/06/2017
 * \brief     Library for opening a TCP server, receiving data and sending message to monitor
 */

#include "server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "image.h"
#include "monitor.h"

#define NB_CONNECTION_MAX 1

int socketFD = -1;
int clientID = -1;

char* imgMsg = NULL;

int openServer(int port) {
    struct sockaddr_in server;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        perror("Can not create socket");
        exit(-1);
    }

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (bind(socketFD, (struct sockaddr *) &server, sizeof (server)) < 0) {
        perror("Can not bind socket");
        exit(-1);
    }

    listen(socketFD, NB_CONNECTION_MAX);

    return socketFD;
}

int closeServer() {
    close(socketFD);

    socketFD = -1;
    
    return 0;
}

int acceptClient() {
    struct sockaddr_in client;
    int c = sizeof (struct sockaddr_in);

    clientID = accept(socketFD, (struct sockaddr *) &client, (socklen_t*) & c);

    if (clientID < 0) {
        perror("Accept failed in acceptClient");
        exit(-1);
    }

    return clientID;
}

int sendDataToServer(char *data, int length) {
    return sendDataToServerForClient(clientID, data, length);
}

int sendDataToServerForClient(int client, char *data, int length) {
    if (client >= 0)
        return write(client, (void*)data, length);
    else return 0;
}

int receiveDataFromServer(char *data, int size) {
    return receiveDataFromServerFromClient(clientID, data, size);
}

int receiveDataFromServerFromClient(int client, char *data, int size) {
    char length = 0;

    if (client > 0) {
        if ((length = recv(client, (void*)data, size, 0)) > 0) {
            data[length] = 0;
        }
    }

    return length;
}

int sendImage(Jpg *image)
{
    int status = 0;
    int lengthSend;
    
    if (imgMsg != NULL) free((void*) imgMsg);
    imgMsg = (char*) malloc(image->size()+ 4);
    imgMsg[0] = HEADER_STM_IMAGE[0];
    imgMsg[1] = HEADER_STM_IMAGE[1];
    imgMsg[2] = HEADER_STM_IMAGE[2];
    imgMsg[3] = ':';

    memcpy((void*) &imgMsg[4], (const void *) reinterpret_cast<char*> (image->data()), image->size());
    
    lengthSend=sendDataToServer(imgMsg, image->size() + 4);

    return status;
}



