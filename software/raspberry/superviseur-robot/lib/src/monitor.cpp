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
 * \file      monitor.cpp
 * \author    L.senaneuch
 * \version   1.0
 * \date      06/06/2017
 * \brief     Library for sending message to monitor or receiving message
 */

#include "monitor.h"
#include "server.h"

using namespace std;

SOCKADDR_IN ssin;
SOCKET sock;
socklen_t recsize = sizeof (ssin);

string serverReceive(int size);
int sendMessage(const char *data, int dataLength);
int receive(char *data);

int send_message_to_monitor(const char* typeMessage, const void * data) {
    if ((string) typeMessage == HEADER_STM_IMAGE) {
        Jpg * imgC = (Jpg*) data;
        sendMessage("IMG", 3);
        sendMessage((const char*) imgC->data(), imgC->size());
        sendMessage("TRAME", 5);
        return 0;
    } else if ((string) typeMessage == HEADER_STM_POS) {
        char buffer[400];
        Position * maPosition = (Position*) data;
        sprintf(buffer, "POScenter: %3d;%3d | %.1fTRAME", maPosition->center.x, maPosition->center.y, maPosition->angle);
        sendMessage(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_MES) {
        char buffer[50];
        sprintf(buffer, "MSG%sTRAME", (const char*) data);
        sendMessage(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_ACK) {
        char buffer[50];
        sprintf(buffer, "ACK%sTRAME", (const char*) data);
        sendMessage(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_BAT) {
        char buffer[50];
        sprintf(buffer, "BAT%sTRAME", (const char*) data);
        sendMessage(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_NO_ACK) {
        char buffer[50];
        sprintf(buffer, "NAK%sTRAME", (const char*) data);
        sendMessage(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_LOST_DMB) {
        char buffer[50];
        sprintf(buffer, "LCD%sTRAME", (const char*) data);
        sendMessage(buffer, strlen(buffer));
        return 0;
    } else {
        return -1;
    }
}

int receive_message_from_monitor(char *typeMessage, char *data) {
    char buffer[20];
    int tBuffer = receive(buffer);
    sscanf(buffer, "%3s:%s", typeMessage, data);
    return tBuffer;
}

int receive(char *data) {
    int result;
    result = receiveDataFromServer(data, 20);
    //cout <<"Data : " << data << endl;
    return result;
}

int sendMessage(const char *data, int dataLength) {
    if (sendDataToServer((char*) data, dataLength) != dataLength) {
        perror("Send failed : ");
        return false;
    }

    return true;
}
