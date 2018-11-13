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
 * \file      monitor.h
 * \author    L.senaneuch
 * \version   1.0
 * \date      06/06/2017
 * \brief     Library for sending message to monitor or receiving message
 */

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image.h"
#include "definitions.h"

#define HEADER_STM_IMAGE "IMG" // Envoi d'une image
#define HEADER_STM_BAT "BAT" // Envoi de l'état de la batterie
#define HEADER_STM_POS "POS" // Envoi de la position
#define HEADER_STM_NO_ACK "NAK" // Acquittement d'un échec
#define HEADER_STM_ACK "ACK" // Acquittement d'un succès
#define HEADER_STM_MES "MSG" // Message textuel
#define HEADER_STM_LOST_DMB "LCD" // Perte de la communication avec le robot

#define HEADER_MTS_MSG "MSG" // Message directe pour Console Dumber
#define HEADER_MTS_DMB_ORDER "DMB" // Message d'ordre pour le robot
#define HEADER_MTS_COM_DMB "COM" // Message de gestion de la communication avec le robot
#define HEADER_MTS_CAMERA "CAM" // Message de gestion de la camera
#define HEADER_MTS_STOP "STO" // Message d'arrêt du system

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define DEFAULT_PORT  8080
#define DEFAULT_PARITY 0

#define DETECT_ARENA    (1)
#define CHECK_ARENA     (2)
#define NO_ARENA        (3)

#define DEFAULT_NODEJS_PATH "/usr/bin/nodejs"
#define DEFAULT_INTERFACE_FILE "./interface.js"

#define closesocket(param) close(param)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

typedef struct {
    char header[4];
    char data[100];
} MessageFromMon;

/**
 * \brief    Envoi d'un message vers l'interface graphique
 * \param    typeMessage     Type du message envoyé. Les valeurs possibles sont 
 * IMG pour une image, MES pour un message à afficher dans la console, POS pour
 * la position du robot, BAT pour une valeur de la batterie et ACK pour valider
 * un message de l'interface.
 * \param   data   données associées au message. Le type de la donnée doit
 * correspondre au message : Image pour IMG, char * MES, Position pour POS, 
 * char * pour BAT et rien pour ACK. Attention, il n'y a aucune vérification
 * a posterio.
 * \return   Retourne 0 si l'envoie a bien été réalisé et -1 en cas de problème.
 */
int send_message_to_monitor(const char* typeMessage, const void * data = NULL);

/**
 * \brief    Réception d'un message. La fonction est bloquante et retourne par
 * référence le type de message reçu (DMB pour un ordre au robot, ARN pour la
 * détection des arènes et POS pour un calcul de position) ainsi que les données
 * associées.
 * \param    typeMessage     Type du message reçu : DMB pour un ordre au robot, 
 * ARN pour la demande de détection de l'arène, POS pour un calcul de position
 * et MSG pour un message de l'interface
 * \param   data   données associées au message reçu.
 * \return   Retourne 0 la taille du message reçu ou une valeur négative si la
 * connexion est perdue.
 */
int receive_message_from_monitor(char *typeMessage, char *data);


#endif // _MONITOR_H_
