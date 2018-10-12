/*******************************************************************************
 * Copyright (c) 2018 INSA - GEI, Toulouse, France.
 * All rights reserved. This program and the accompanying materials
 * are made available "AS IS", without any warranty of any kind.
 *
 * INSA assumes no responsibility for errors or omissions in the 
 * software or documentation available. 
 *
 * Contributors:
 *     Lucien Senaneuch - Initial API and implementation
 *     Sebastien DI MERCURIO - Maintainer since Octobre 2018
 *******************************************************************************/
/**
 * \file      robot.h
 * \author    L.Senaneuch
 * \version   1.0
 * \date      06/06/2017
 * \brief     Fonction permettant la communication avec le robot.
 *
 * \details   Ce fichier regroupe des fonctions facilitant la communication avec le robot en utilisant le port serie USART
 */

#ifndef __ROBOT_H__
#define __ROBOT_H__

#include <iostream>
#include <string>
#include <Serial.h>

using namespace std;

#define REJECTED_COMMAND                    -3
#define INVALID_COMMAND                     -4
#define TIMEOUT_COMMAND                     -5
#define INVALID_ANSWER                      -6

#define BATTERY_FULL                        2
#define BATTERY_LOW                         1
#define BATTERY_EMPTY                       0

class Robot : public Serial {
    public:
        Robot();
        virtual ~Robot();

        char Open();
        char Open(const char *path);
        char Close();

        int GetLastCommandStatus();

        void Ping();
        void Reset();

        void StartWithoutWatchdog();
        void StartWithWatchdog();
        void ResetWatchdog();

        void Move(int distance);
        void Turn(int angle);

        void PowerOff();

        char GetBatteryLevel();
        string GetVersion();
        bool IsBusy();

        // Ces deux methodes virtuelles sont a redefinir (surcharger) dans une classe qui etends la classe Robot
        // Servez vous en pour faire une action avant (prologue) ou apres (epilogue) une commande au robot
        virtual void CommunicationProlog();
        virtual void CommunicationEpilog();

        static const string InvalidAnswerException;
    private:

        int lastCommandStatus;

        char SendCommand(string cmd, string *ans);
        string AddChecksum(string cmd);
        bool ValidateChecksum(string *answerWithoutChecksum, string answer);
        void CheckAnswer(string ans);
};

#endif //__ROBOT_H__
