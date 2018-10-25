/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server.h
 * Author: dimercur
 *
 * Created on 19 octobre 2018, 14:24
 */

#ifndef _SERVER_H_
#define _SERVER_H_

#define DEFAULT_SERVER_PORT 2323

int openServer (int port);
int closeServer();
int acceptClient();

int sendDataToServer(char *data, int length);
int sendDataToServerForClient(int client, char *data, int length);
int receiveDataFromServer(char *data, int size);
int receiveDataFromServerFromClient(int client, char *data, int size);
#endif /* _SERVER_H_ */

