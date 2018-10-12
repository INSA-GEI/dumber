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
 * \file      robot.cpp
 * \author    L.Senaneuch
 * \version   1.0
 * \date      06/06/2017
 * \brief     Fonction permettant la communication avec le robot.
 *
 * \details   Ce fichier regroupe des fonctions facilitant la communication avec le robot en utilisant le port serie USART
 */

#include "Robot.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <stdlib.h>
#include "definitions.h"

using namespace std;

#define ROBOT_CMD_ENDING_CHAR           '\r'

#define ROBOT_CMD_PING                  'p'
#define ROBOT_CMD_RESET                 'r'
#define ROBOT_CMD_START_WITHOUT_WD      'u'
#define ROBOT_CMD_START_WITH_WD         'W'
#define ROBOT_CMD_RELOAD_WD             'w'
#define ROBOT_CMD_GET_VBAT              'v'
#define ROBOT_CMD_IS_BUSY               'b'
#define ROBOT_CMD_MOVE                  'M'
#define ROBOT_CMD_TURN                  'T'
#define ROBOT_CMD_GET_VERSION           'V'
#define ROBOT_CMD_POWER_OFF             'z'

#define ROBOT_CMD_OK_ANS                "O"
#define ROBOT_CMD_ERR_ANS               "E"
#define ROBOT_CMD_UNKNOW_ANS            "C"

const string Robot::InvalidAnswerException = "Invalid answer exception";

Robot::Robot() {

}

Robot::~Robot() {
    this->Close();
}

char Robot::Open() {
    return Serial::Open(DEFAULT_SERIAL_PORT,9600);
}

char Robot::Open(const char *path) {
    return Serial::Open(path,9600);
}

char Robot::Close() {
    return Serial::Close();
}

int Robot::GetLastCommandStatus() {
    return this->lastCommandStatus;
}

void Robot::Ping() {
#ifndef __STUB__
    string ans;
    SendCommand(string(1,ROBOT_CMD_PING), &ans);

    CheckAnswer(ans);
#else
    return SUCCESS;
#endif /* __STUB__ */
}

void Robot::Reset() {
#ifndef __STUB__
    string ans;
    SendCommand(string(1,ROBOT_CMD_RESET), &ans);

    CheckAnswer(ans);
#else
    return SUCCESS;
#endif /* __STUB__ */
}

void Robot::StartWithoutWatchdog() {
#ifndef __STUB__
    string ans;
    SendCommand(string(1,ROBOT_CMD_START_WITHOUT_WD), &ans);

    CheckAnswer(ans);
#else
    return SUCCESS;
#endif /* __STUB__ */
}

void Robot::StartWithWatchdog() {
#ifndef __STUB__
    string ans;
    SendCommand(string(1,ROBOT_CMD_START_WITH_WD), &ans);

    CheckAnswer(ans);
#else
    return SUCCESS;
#endif /* __STUB__ */
}

void Robot::ResetWatchdog() {
#ifndef __STUB__
    string ans;
    SendCommand(string(1,ROBOT_CMD_RELOAD_WD), &ans);

    CheckAnswer(ans);
#else
    return SUCCESS;
#endif /* __STUB__ */
}

void Robot::Move(int distance) {
#ifndef __STUB__
    string ans;
    SendCommand(string(1,ROBOT_CMD_MOVE) + "="+to_string(distance), &ans);

    CheckAnswer(ans);
#else
    return SUCCESS;
#endif /* __STUB__ */
}

void Robot::Turn(int angle) {
#ifndef __STUB__
    string ans;
    SendCommand(string(1,ROBOT_CMD_TURN) + "="+to_string(angle), &ans);

    CheckAnswer(ans);
#else
    return SUCCESS;
#endif /* __STUB__ */
}

void Robot::PowerOff() {
#ifndef __STUB__
    string ans;
    SendCommand(string(1,ROBOT_CMD_POWER_OFF), &ans);

    CheckAnswer(ans);
#else
    return SUCCESS;
#endif /* __STUB__ */
}

char Robot::GetBatteryLevel() {
#ifndef __STUB__
    string ans;

    SendCommand(string(1,ROBOT_CMD_GET_VBAT), &ans);
    lastCommandStatus=SUCCESS;

    if (ans.length()==1) {
        if ((ans[0] != '2') && (ans[0] != '1') && (ans[0] != '0')) {
            lastCommandStatus=INVALID_ANSWER;
            throw (InvalidAnswerException + " raised in Robot::GetBatteryLevel. Invalid battery value (" + ans[0] +")\n");
        }
    } else {
        lastCommandStatus=INVALID_ANSWER;
        throw (InvalidAnswerException + " raised in Robot::GetBatteryLevel. Invalid answer length (" + to_string(ans.length())+")\n");
    }

    return ans[0];
#else
    return BATTERY_FULL;
#endif /* __STUB__ */
}

string Robot::GetVersion() {
#ifndef __STUB__
    string ans;

    SendCommand(string(1,ROBOT_CMD_GET_VERSION), &ans);
    lastCommandStatus=SUCCESS;

    if (ans.find("version")== string::npos) {
        lastCommandStatus=INVALID_ANSWER;
        throw (InvalidAnswerException + " raised in Robot::GetVersion. Invalid version (" + ans +")\n");
    }

    return ans;
#else
    return "1.3";
#endif /* __STUB__ */
}

bool Robot::IsBusy() {
#ifndef __STUB__
    string ans;

    SendCommand(string(1,ROBOT_CMD_IS_BUSY), &ans);
    lastCommandStatus=SUCCESS;

    if (ans.length()!=1) {
        lastCommandStatus=INVALID_ANSWER;
        throw (InvalidAnswerException + " raised in Robot::GetBatteryLevel. Invalid answer length (" + to_string(ans.length())+")\n");
    }

    if (ans[0]=='1')
        return true;
    else if (ans[0]=='0')
        return false;
    else {
        lastCommandStatus=INVALID_ANSWER;
        throw (InvalidAnswerException + " raised in Robot::IsBusy. Invalid value (" + ans[0] +")\n");
    }
#else
    return false;
#endif /* __STUB__ */
}

void Robot::CommunicationProlog() {}
void Robot::CommunicationEpilog() {}

/****************************/
/* PRIVATE                  */
/****************************/

char Robot::SendCommand(string cmd, string *ans) {
    string commandString;
    commandString = this->AddChecksum(cmd);

    this->CommunicationProlog(); // action avant le debut de la commande

    ssize_t status = Send(commandString);

    if (status == (ssize_t)commandString.length()) {
        // Recuperation de la reponse
        string commandResponse;
        vector<char> endingChars;

        endingChars.push_back('\r');
        try {
            commandResponse = Serial::Receive(endingChars,30);
        } catch (string e) {
            if (e.find("Timeout")!= string::npos)
                lastCommandStatus= TIMEOUT_COMMAND;
            else
                lastCommandStatus=INVALID_ANSWER;

            if (lastCommandStatus == TIMEOUT_COMMAND)
                throw (Serial::TimeoutException + " raised in Robot::SendCommand. Timeout while receiving answer from robot.\n");
            else
                throw (e + " raised in Robot::SendCommand.\n");
        }

        this->CommunicationEpilog(); // Action a faire apres la commande

        if (ValidateChecksum(&commandResponse, commandResponse)) {
            ans->assign(commandResponse);
        } else {
            lastCommandStatus = INVALID_ANSWER;
            throw InvalidAnswerException;
        }

        lastCommandStatus=SUCCESS;
    } else {
        lastCommandStatus=INVALID_COMMUNICATION_PORT;

        throw IOErrorException;
    }

    return lastCommandStatus;
}

void Robot::CheckAnswer(string ans) {

    if (ans.length()==1)
    {
        switch (ans.at(0)) {
            case 'o':
            case 'O':
                lastCommandStatus=SUCCESS;
                break;
            case 'e':
            case 'E':
                lastCommandStatus=REJECTED_COMMAND;
                break;
            case 'c':
            case 'C':
                lastCommandStatus=INVALID_COMMAND;
                break;
            default:
                lastCommandStatus = INVALID_ANSWER;
                throw InvalidAnswerException;
        }
    } else {
        lastCommandStatus = INVALID_ANSWER;
        throw InvalidAnswerException;
    }
}

string Robot::AddChecksum(string cmd) {
    string commandWithChecksum(cmd);
    char checksum=0;

    commandWithChecksum.resize(cmd.length()+2, ' ');

    for (size_t i=0; i<commandWithChecksum.length()-2; i++) {
        checksum = checksum^commandWithChecksum.at(i);
    }

    commandWithChecksum.at(commandWithChecksum.length()-2) = checksum;
    commandWithChecksum.at(commandWithChecksum.length()-1) = '\r';

    return commandWithChecksum;
}

bool Robot::ValidateChecksum(string *answerWithoutChecksum, string answer) {
    string localAnswer(answer, 0, answer.length()-1); // recopie de la chaine initiale, sans le dernier caractere (checksum)
    char checksum=0;
    bool status=false;

    if (answer.length()<2) // trop court pour contenir un checksum
        throw InvalidAnswerException;
    else {
        for (size_t i=0; i< answer.length()-1; i++) {
            checksum = checksum^answer.at(i);
        }

        if (answer.at(answer.length()-1) == checksum) {
            status=true;
        }
    }

    answerWithoutChecksum->assign(localAnswer);
    return status;
}
