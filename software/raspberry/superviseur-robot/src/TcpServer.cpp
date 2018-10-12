/*******************************************************************************
 * Copyright (c) 2018 INSA - GEI, Toulouse, France.
 * All rights reserved. This program and the accompanying materials
 * are made available "AS IS", without any warranty of any kind.
 *
 * INSA assumes no responsibility for errors or omissions in the 
 * software or documentation available. 
 *
 * Part of code Copyright ST Microelectronics.
 *
 * Contributors:
 *     Lucien Senaneuch - Initial API and implementation
 *     Sebastien DI MERCURIO - Maintainer since Octobre 2018
 *******************************************************************************/


#include "TcpServer.h"
#include <netinet/in.h>
#include <zconf.h>
#include <vector>

TcpServer::TcpServer() {
    this->socketFD = -1;
    this->socketClients.clear();
}

int TcpServer::Listen (int port) {
    struct sockaddr_in server;

    this->socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(this->socketFD < 0){
        throw invalid_argument("Can not create socket");
    }

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if(bind(this->socketFD, (struct sockaddr *) &server, sizeof(server)) < 0) {
        throw invalid_argument("Can not bind socket");
    }

    listen(this->socketFD , NB_CONNECTION_MAX);

    return this->socketFD;
}

int TcpServer::AcceptClient() {
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);

    int fd = accept(this->socketFD,(struct sockaddr *) &client, (socklen_t*)&c);

    if (fd >=0 )
        this->socketClients.push_back(fd);
    else throw invalid_argument("Accept failed in TcpServer::AcceptClient");

    return fd;
}

int TcpServer::Send(int client, string mes) {
    return write(client, mes.c_str(), mes.size());
}

string TcpServer::Receive(int client_fd, int size){
    char tab[size];

    if(recv(client_fd,tab,size,0) >0) {
        tab[size] = 0;
        return string(tab);
    } else
        return string();
}

int TcpServer::Broadcast(string mes) {
    for (int socket : this->socketClients) {
        int err = write(socket, mes.c_str(), mes.size());
    }

    return 0;
}

const vector<int> &TcpServer::getSocketClients() const {
    return socketClients;
}

void TcpServer::SetSocketClients(const std::vector<int> &socketClients) {
    this->socketClients = socketClients;
}

TcpServer::~TcpServer() {
    close(this->socketFD);
}
