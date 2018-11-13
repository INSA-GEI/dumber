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
 * \file      server.h
 * \author    PE.Hladik
 * \version   1.0
 * \date      06/06/2017
 * \brief     Library for opening a TCP server, receiving data and sending message to monitor
 */

#ifndef _SERVER_H_
#define _SERVER_H_

#include "image.h"
#define DEFAULT_SERVER_PORT 2323

/**
 * \brief       Open server port, connect and listen to given port.
 *       
 * \param       port A valid port number (1024 - 65535)      
 * \return      -1 if opening failed or the socket number
 */ 
int openServer (int port);

/**
 * \brief       Close server
 *             
 * \return      -1 if closing failed , 0 otherwise
 */ 
int closeServer();

/**
 * \brief       Wait for a client to connect
 *             
 * \return      Return client Id or -1 if it failed
 */ 
int acceptClient();

/**
 * \brief       Send given data to monitor
 * \details     Send given data to monitor using default client ID
 *             
 * \param       data A valid pointer to a buffer  
 * \param       length Amount of data to send  
 * \return      Return amount of data really written. 0 if communication is broken
 */ 
int sendDataToServer(char *data, int length);

/**
 * \brief       Send given data to monitor, using specific client ID
 * \details     Send given data to monitor using given client ID.
 *       
 * \param       client Client Id to send data to  
 * \param       data A valid pointer to a buffer  
 * \param       length Amount of data to send        
 * \return      Return amount of data really written. 0 if communication is broken
 */ 
int sendDataToServerForClient(int client, char *data, int length);

/**
 * \brief       Read data from monitor.
 * \details     Read, at most, size data from monitor. Data must be a valid pointer to a buffer large enough.
 *           
 * \param       data A valid pointer to a buffer  
 * \param       size Amount of data to read    
 * \return      Return amount of data really received. 0 if communication is broken
 */ 
int receiveDataFromServer(char *data, int size);

/**
 * \brief       Read data from monitor, using specific client ID.
 * \details     Read, at most, size data from monitor. Data must be a valid pointer to a buffer large enough. 
 * 
 * \param       client Client Id to receive from 
 * \param       data A valid pointer to a buffer  
 * \param       size Amount of data to read              
 * \return      Return amount of data really received. 0 if communication is broken
 */ 
int receiveDataFromServerFromClient(int client, char *data, int size);

/**
 * \brief       Send image to monitor using default client ID
 * \details     Convert image to raw data, and add correct header before sending to monitor
 * 
 * \param       image An image object after compression        
 * \return      Return amount of data really received. 0 if communication is broken
 */ 
int sendImage(Jpg *image);
#endif /* _SERVER_H_ */

