/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server.cpp
 * Author: dimercur
 * 
 * Created on 19 octobre 2018, 14:24
 */

#include "server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NB_CONNECTION_MAX 1

int socketFD = -1;
int clientID = -1;

int socketUDP= -1;
struct sockaddr_in UDPcliaddr;
int UDPcliaddrlen = -1;
char *UDPBuffer=0;

int openServer(int port) {
    struct sockaddr_in server;
    struct sockaddr_in serverUDP;
    
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

    /* Open UDP connection */
    socketUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketUDP < 0) {
        perror("Can not create UDP socket");
        exit(-1);
    }
    
    serverUDP.sin_addr.s_addr = INADDR_ANY;
    serverUDP.sin_family = AF_INET;
    serverUDP.sin_port = htons(port+1);
    
    memset(&UDPcliaddr, 0, sizeof(UDPcliaddr)); 
    
    if (bind(socketUDP, (struct sockaddr *) &serverUDP, sizeof (serverUDP)) < 0) {
        perror("Can not bind UDP socket");
        exit(-1);
    }
    
    return socketFD;
}

int closeServer() {
    close(socketFD);
    close(socketUDP);
    
    socketFD = -1;
    socketUDP = -1;
    
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

void waitUDPClientAddr(void)
{
    char buffer[10]; 
    recvfrom(socketUDP, (char *)buffer, 10,  
            MSG_WAITALL, ( struct sockaddr *) &UDPcliaddr, &UDPcliaddrlen); 
}

int sendImage(char *data, int length) {
    if (clientID >= 0)
    {
        UDPBuffer= (char*)malloc(length+12);
    
        UDPBuffer[0]='I';
        UDPBuffer[1]='N';
        UDPBuffer[2]='S';
        UDPBuffer[3]='A';
        UDPBuffer[4]='B';
        UDPBuffer[5]='G';
        
        UDPBuffer[length+6]='A';
        UDPBuffer[length+7]='S';
        UDPBuffer[length+8]='N';
        UDPBuffer[length+9]='I';
        UDPBuffer[length+10]='E';
        UDPBuffer[length+11]='D';
        
        memcpy((void*)(UDPBuffer+6),(const void *)data, length);
        
        return sendto(socketUDP, data, length, 
        MSG_CONFIRM, (const struct sockaddr *) &UDPcliaddr, UDPcliaddrlen); 
    }   
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




