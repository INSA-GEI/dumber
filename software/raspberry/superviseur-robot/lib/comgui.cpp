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

#include "comgui.h"
#include <iostream>
#include <sys/socket.h>

#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <algorithm>
#include <stdexcept>
#include <string>

/*
 * Constants used for sending commands to gui
 */
const string LABEL_GUI_ANGULAR_POSITION = "AngularPosition";
const string LABEL_GUI_ANGULAR_SPEED = "AngularSpeed";
const string LABEL_GUI_BATTERY_LEVEL = "Battery";
const string LABEL_GUI_LINEAR_SPEED = "LinearSpeed";
const string LABEL_GUI_USER_PRESENCE = "User";
const string LABEL_GUI_BETA_ANGLE = "Beta";
const string LABEL_GUI_TORQUE = "Torque";
const string LABEL_GUI_EMERGENCY_STOP = "Emergency";
const string LABEL_GUI_LOG = "Log";

/**
 * Create a server and open a socket over TCP
 * 
 * @param port Port used for communication
 * @return Socket number
 * @throw std::runtime_error if it fails
 */
int ComGui::Open(int port) {
    struct sockaddr_in server;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        throw std::runtime_error{"ComGui::Open : Can not create socket"};
    }

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (bind(socketFD, (struct sockaddr *) &server, sizeof (server)) < 0) {
        throw std::runtime_error{"ComGui::Open : Can not bind socket on port " + std::to_string(port)};
    }

    listen(socketFD, 1);

    return socketFD;
}

/**
 * Close socket and server
 */
void ComGui::Close() {
    close(socketFD);

    socketFD = -1;
}

/**
 * Wait for a client to connect
 * @return Client number 
 * @throw std::runtime_error if it fails
 */
int ComGui::AcceptClient() {
    struct sockaddr_in client;
    int c = sizeof (struct sockaddr_in);

    clientID = accept(socketFD, (struct sockaddr *) &client, (socklen_t*) & c);

    if (clientID < 0)
        throw std::runtime_error {
        "ComGui::AcceptClient : Accept failed"
    };

    return clientID;
}

/**
 * Send a message to GUI
 * 
 * @param msg Message to send to GUI
 * @attention Message given in parameter will be destroyed (delete) after being sent. No need for user to delete message after that.
 * @warning Write is not thread safe : check that multiple tasks can't access this method simultaneously  
 */
void ComGui::Write(Message* msg) {
    string *str;

    // Call user method before Write
    Write_Pre();

    /* Convert message to string to send to GUI */
    str = MessageToString(msg);

    //cout << "Message sent to GUI: " << str->c_str() << endl;
    write(clientID, str->c_str(), str->length());

    delete(str);

    // Call user method after write
    Write_Post();
}

/**
 * Method used internally to convert a message content to a string that can be sent over TCP
 * @param msg Message to be converted
 * @return A string, image of the message
 */
string *ComGui::MessageToString(Message *msg) {
    int id;
    string *str;

    if (msg != NULL) {
        id = msg->GetID();

        switch (id) {
            case MESSAGE_ANGLE_POSITION:
                str = new string(LABEL_GUI_ANGULAR_POSITION + "=" + to_string(((MessageFloat*) msg)->GetValue()) + "\n");
                replace(str->begin(), str->end(), '.', ','); // Mono C# require float to have a , instead of a .
                break;
            case MESSAGE_ANGULAR_SPEED:
                str = new string(LABEL_GUI_ANGULAR_SPEED + "=" + to_string(((MessageFloat*) msg)->GetValue()) + "\n");
                replace(str->begin(), str->end(), '.', ','); // Mono C# require float to have a , instead of a .
                break;
            case MESSAGE_BATTERY:
                str = new string(LABEL_GUI_BATTERY_LEVEL + "=" + to_string(((MessageFloat*) msg)->GetValue()) + "\n");
                replace(str->begin(), str->end(), '.', ','); // Mono C# require float to have a , instead of a .
                break;
            case MESSAGE_BETA:
                str = new string(LABEL_GUI_BETA_ANGLE + "=" + to_string(((MessageFloat*) msg)->GetValue()) + "\n");
                replace(str->begin(), str->end(), '.', ','); // Mono C# require float to have a , instead of a .
                break;
            case MESSAGE_LINEAR_SPEED:
                str = new string(LABEL_GUI_LINEAR_SPEED + "=" + to_string(((MessageFloat*) msg)->GetValue()) + "\n");
                replace(str->begin(), str->end(), '.', ','); // Mono C# require float to have a , instead of a .
                break;
            case MESSAGE_TORQUE:
                str = new string(LABEL_GUI_TORQUE + "=" + to_string(((MessageFloat*) msg)->GetValue()) + "\n");
                replace(str->begin(), str->end(), '.', ','); // Mono C# require float to have a , instead of a .
                break;
            case MESSAGE_EMERGENCY_STOP:
                str = new string(LABEL_GUI_EMERGENCY_STOP + "=");
                if (((MessageBool*) msg)->GetState())
                    str->append("True\n");
                else
                    str->append("False\n");
                break;
            case MESSAGE_USER_PRESENCE:
                str = new string(LABEL_GUI_USER_PRESENCE + "=");
                if (((MessageBool*) msg)->GetState())
                    str->append("True\n");
                else
                    str->append("False\n");
                break;
            case MESSAGE_EMPTY:
                str = new string(""); //empty string
                break;
            case MESSAGE_LOG:
                str = new string(LABEL_GUI_LOG + "=" + ((MessageString*) msg)->GetString() + "\n");
                break;
            default:
                str = new string(""); //empty string
                break;
        }
    }

    return str;
}
