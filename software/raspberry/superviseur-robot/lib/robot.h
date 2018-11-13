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
 * \file      robot.h
 * \author    L.Senaneuch
 * \version   1.0
 * \date      06/06/2017
 * \brief     Fonctions for communicating with robot.
 */

#ifndef _ROBOT_H_
#define _ROBOT_H_

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include "definitions.h"

#ifdef __FOR_PC__
#define serialPort "/dev/ttyUSB0"
#else
#define serialPort "/dev/ttyS0"
#endif /* __FOR_PC__ */

typedef struct
   {
    char header[4];
    char data[20];
   } MessageToRobot;

 /**
 * \brief       Ouvre la communication avec le robot.
 * \details     Ouvre le serial port passé en paramétre. Par defaut cette fonction ouvre le port ttySO connecté au module xbee.
 *             
 * \param       path    Chaine de caractère contenant le path du port serie à ouvrir.
 * \return      Return -1 si l'ouverture c'est mal passé et 0 si le port est ouvert.
 */
int open_communication_robot(const char * path=serialPort);

 /**
 * \brief       Ferme la communication avec le robot.
 * \details     Ferme le descripteur de fichier du port serie contrôlant le robot.
 *             
 * \return      Retourne -1 en cas d'erreur ou 0 en cas de fermeture effectué
 */ 
int close_communication_robot(void);

 /**
 * \brief       Envoi une commande au robot et attends sa réponse.
 * \details     Envoi une commande au robot en ajoutant le checksum et lis la réponse du robot en verifiant le checksum.
			   Le premier paramétre \a cmd correspond au type de commande ex : PING, SETMOVE ...
			   Le second paramétre  \a *arg correspond aux arguments à la commande ex : SETMOVE, "100"
			   La fonction retourne un code confirmation transmise par le robot (ROBOT_CHEKSUM, ROBOT_ERROR, ROBOT_TIMED_OUT, ROBOT_OK, ROBOT_UKNOW_CMD)
 *             
 * \param       cmd    Entête de la commande
 * \param       arg   Argument de la commande 
 * \return      Retourne un code confirmation.
 */
int send_command_to_robot(char cmd, const char * arg=NULL);

#endif //_ROBOT_H_
