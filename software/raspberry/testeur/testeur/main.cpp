/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: dimercur
 *
 * Created on 6 novembre 2018, 10:54
 */

#include <cstdlib>

#include "image.h"
#include "server.h"
#include "robot.h"
#include "message.h"

#include <iostream>
#include <string>
#include <time.h>
#include <thread>

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

int socketID;
char data[1000];
int receivedLength;
bool disconnected = true;
bool dataReady = false;
bool sysTick = false;
bool sendImage = false;
bool sendPos = false;

Image monImage;
Jpg imageCompressed;

pthread_t thread;

typedef struct {
    char header[4];
    char data[500];
} MessageFromMon;

MessageFromMon *message;
MessageToMon messageAnswered;

std::thread *threadTimer;
std::thread *threadServer;

using namespace std;

/*
 * 
 */
void ThreadServer(void) {
    // Recuperation d'une evenutelle commande sur le serveur
    while (1) {
        receivedLength = receiveDataFromServer(data, 1000);
        if (receivedLength > 0) dataReady = true;
    }
}

void ThreadTimer(void) {
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 100000000;

    while (1) {
        //std::this_thread::sleep_for(std::chrono::seconds )
        //sleep(1);
        if (nanosleep(&tim, &tim2) < 0) {
            printf("Nano sleep system call failed \n");
            return;
        }
        
        sysTick = true;
    }
}

void printReceivedMessage(MessageFromMon *mes) {
    cout << "Received " + to_string(receivedLength) + " data\n";
    cout << "Header: ";

    for (int i = 0; i < 4; i++) {
        cout << mes->header[i];
    }

    cout << "\nData: ";
    for (int i = 0; i < receivedLength - 4; i++) {
        cout << mes->data[i];
    }

    cout << "\n";
}

int sendAnswer(string cmd, string data) {
    int status = 0;
    string msg;

    msg = cmd + ':' + data;
    cout << "Answer: " + msg;
    cout << "\n";
    sendDataToServer((char*) msg.c_str(), msg.length());

    return status;
}

int sendBinaryData(string cmd, char* data, int length) {
    int status = 0;
    char* msg;

    msg = (char*) malloc(length + 4);
    msg[0] = cmd[0];
    msg[1] = cmd[1];
    msg[2] = cmd[2];
    msg[3] = ':';

    memcpy((void*) &msg[4], (const void *) data, length);
    cout << "Answer: " + cmd;
    cout << "\n";
    sendDataToServer(msg, length + 4);

    free((void*) msg);
    return status;
}

int decodeMessage(MessageFromMon *mes, int dataLength) {
    int status = 0;
    string header(mes->header, 4);
    string data(mes->data, dataLength);

    if (header.find(HEADER_MTS_COM_DMB) != std::string::npos) // Message pour la gestion du port de communication
    {
        if (data.find(OPEN_COM_DMB) != std::string::npos) sendAnswer(HEADER_STM_ACK, "");
        else if (data.find(CLOSE_COM_DMB) != std::string::npos) sendAnswer(HEADER_STM_ACK, "");
    } else if (header.find(HEADER_MTS_CAMERA) != std::string::npos) // Message pour la camera
    {
        if (data.find(CAM_OPEN) != std::string::npos) {
            sendAnswer(HEADER_STM_ACK, "");
            sendImage = true;
        } else if (data.find(CAM_CLOSE) != std::string::npos) {
            sendImage = false;
        } else if (data.find(CAM_COMPUTE_POSITION) != std::string::npos) {
            sendPos = true;
        } else if (data.find(CAM_STOP_COMPUTE_POSITION) != std::string::npos) {
            sendPos = false;
        } else {

        }
    } else if (header.find(HEADER_MTS_DMB_ORDER) != std::string::npos) // Message de com pour le robot
    {
        if (data.find(DMB_START_WITHOUT_WD) != std::string::npos) {
            sendAnswer(HEADER_STM_ACK, "");

        } else if (data.find(DMB_START_WITH_WD) != std::string::npos) {
            sendAnswer(HEADER_STM_ACK, "");

        } else if (data.find(DMB_GET_VBAT) != std::string::npos) {
            sendAnswer(HEADER_STM_BAT, to_string(DMB_BAT_HIGH));
        } else if (data.find(DMB_MOVE) != std::string::npos) {

        } else if (data.find(DMB_TURN) != std::string::npos) {

        } else {

        }
    } else if (header.find(HEADER_MTS_STOP) != std::string::npos) // Message d'arret
    {
        sendAnswer(HEADER_STM_ACK, "");
    } else {
        sendAnswer(HEADER_STM_NO_ACK, "");
    }

    return status;
}

int main(int argc, char** argv) {

    namedWindow("Sortie Camera");

    // Ouverture de la com robot
    /*if (open_communication_robot("/dev/ttyUSB0") != 0) {
        cerr << "Unable to open /dev/ttyUSB0: abort\n";
        return -1;
    }
    cout << "/dev/ttyUSB0 opened\n";
     */
    // Ouverture de la camera
    if (open_camera(0) == -1) {
        cerr << "Unable to open camera: abort\n";
        return -1;
    }
    cout << "Camera opened\n";

    // Ouverture du serveur
    socketID = openServer(5544);
    cout << "Server opened on port 5544\n";

    threadTimer = new std::thread(ThreadTimer);

    for (;;) {
        cout << "Waiting for client to connect ...\n";
        acceptClient();
        disconnected = false;
        dataReady = false;
        cout << "Client connected\n";
        threadServer = new std::thread(ThreadServer);

        while (disconnected == false) {

            // Recuperation de l'image
            get_image(0, &monImage, "");

            if (dataReady == true) // des données ont été recu par le serveur
            {
                message = (MessageFromMon*) malloc(sizeof (MessageFromMon));
                memcpy((void*) message, (const void*) data, sizeof (MessageFromMon));
                dataReady = false;

                //if (message->header[4] == ':') message->header[4];
                printReceivedMessage(message);
                decodeMessage(message, receivedLength - 4);

                free(message);
            }

            if (sysTick) {
                sysTick = false;

                if (sendImage) {
                    compress_image(&monImage, &imageCompressed);
                    int length = imageCompressed.size();
                    sendBinaryData(HEADER_STM_IMAGE, reinterpret_cast<char*> (imageCompressed.data()), length);
                    //sendAnswer(HEADER_STM_IMAGE, reinterpret_cast<char*> (imageCompressed.data()));
                }

                if (sendPos) {
                    //sendAnswer(HEADER_STM_POS,)
                }
            }
        }
    }

    threadTimer->join();
    threadServer->join();

    // test de la camera
    if (open_camera(0) != -1) {
        for (;;) {
            get_image(0, &monImage, "");

            if (monImage.empty()) printf("image vide");
            else {
                imshow("Sortie Camera", monImage);

                waitKey(10);
            }
        }
    } else {
        printf("Echec ouverture de camera");
        return -1;
    }

    return 0;
}

