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

#include "commonitor.h"
#include <iostream>
#include <sys/socket.h>

#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <algorithm>
#include <stdexcept>
#include <string>

#include "base64/base64.h"

/*
 * @brief Constants used for sending commands to monitor
 */
const string LABEL_MONITOR_ANSWER_ACK = "AACK";
const string LABEL_MONITOR_ANSWER_NACK = "ANAK";
const string LABEL_MONITOR_ANSWER_COM_ERROR = "ACER";
const string LABEL_MONITOR_ANSWER_TIMEOUT = "ATIM";
const string LABEL_MONITOR_ANSWER_CMD_REJECTED = "ACRJ";
const string LABEL_MONITOR_MESSAGE = "MSSG";
const string LABEL_MONITOR_CAMERA_OPEN = "COPN";
const string LABEL_MONITOR_CAMERA_CLOSE = "CCLS";
const string LABEL_MONITOR_CAMERA_IMAGE = "CIMG";
const string LABEL_MONITOR_CAMERA_ARENA_ASK = "CASA";
const string LABEL_MONITOR_CAMERA_ARENA_INFIRM = "CAIN";
const string LABEL_MONITOR_CAMERA_ARENA_CONFIRM = "CACO";
const string LABEL_MONITOR_CAMERA_POSITION_COMPUTE = "CPCO";
const string LABEL_MONITOR_CAMERA_POSITION_STOP = "CPST";
const string LABEL_MONITOR_CAMERA_POSITION = "CPOS";
const string LABEL_MONITOR_ROBOT_COM_OPEN = "ROPN";
const string LABEL_MONITOR_ROBOT_COM_CLOSE = "RCLS";
const string LABEL_MONITOR_ROBOT_PING = "RPIN";
const string LABEL_MONITOR_ROBOT_RESET = "RRST";
const string LABEL_MONITOR_ROBOT_START_WITHOUT_WD = "RSOW";
const string LABEL_MONITOR_ROBOT_START_WITH_WD = "RSWW";
const string LABEL_MONITOR_ROBOT_RELOAD_WD = "RLDW";
const string LABEL_MONITOR_ROBOT_MOVE = "RMOV";
const string LABEL_MONITOR_ROBOT_TURN = "RTRN";
const string LABEL_MONITOR_ROBOT_GO_FORWARD = "RGFW";
const string LABEL_MONITOR_ROBOT_GO_BACKWARD = "RGBW";
const string LABEL_MONITOR_ROBOT_GO_LEFT = "RGLF";
const string LABEL_MONITOR_ROBOT_GO_RIGHT = "RGRI";
const string LABEL_MONITOR_ROBOT_STOP = "RSTP";
const string LABEL_MONITOR_ROBOT_POWEROFF = "RPOF";
const string LABEL_MONITOR_ROBOT_BATTERY_LEVEL = "RBLV";
const string LABEL_MONITOR_ROBOT_GET_BATTERY = "RGBT";
const string LABEL_MONITOR_ROBOT_GET_STATE = "RGST";
const string LABEL_MONITOR_ROBOT_CURRENT_STATE = "RCST";

const string LABEL_SEPARATOR_CHAR = ":";

/**
 * Create a server and open a socket over TCP
 * 
 * @param port Port used for communication
 * @return Socket number
 * @throw std::runtime_error if it fails
 */
int ComMonitor::Open(int port) {
    struct sockaddr_in server;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        throw std::runtime_error{"Can not create socket"};
    }

    int enable = 1;
    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        cerr<<"setsockopt(SO_REUSEADDR) failed"<<endl<<flush;
    }
    
    bzero((char *) &server, sizeof(server));
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (bind(socketFD, (struct sockaddr *) &server, sizeof (server)) < 0) {
        cerr<<"["<<__PRETTY_FUNCTION__<<"] Can not bind socket ("<<to_string(port)<<")"<<endl<<flush;
        throw std::runtime_error{"Can not bind socket"};
    }

    listen(socketFD, 1);

    return socketFD;
}

/**
 * Close socket and server
 */
void ComMonitor::Close() {
    close(socketFD);

    socketFD = -1;
}

/**
 * Wait for a client to connect
 * @return Client number 
 * @throw std::runtime_error if it fails
 */
int ComMonitor::AcceptClient() {
    struct sockaddr_in client;
    int c = sizeof (struct sockaddr_in);

    clientID = accept(socketFD, (struct sockaddr *) &client, (socklen_t*) & c);

    if (clientID < 0)
        throw std::runtime_error {"Accept failed"};

    return clientID;
}

/**
 * Send a message to monitor
 * 
 * @param msg Message to send to monitor
 * @attention Message given in parameter will be destroyed (delete) after being sent. No need for user to delete message after that.
 * @warning Write is not thread safe : check that multiple tasks can't access this method simultaneously  
 */
void ComMonitor::Write(Message *msg) {
    string str;
    
    // Call user method before Write
    Write_Pre();

    /* Convert message to string to send to monitor */
    str = MessageToString(msg);

    //cout << "Message sent to monitor: " << str->c_str() << endl;
    write(clientID, str.c_str(), str.length());

    if (!msg->CompareID(MESSAGE_CAM_IMAGE)) {
        delete(msg);
    }
   
    // Call user method after write
    Write_Post();
}

/**
 * Receive a message from monitor
 * 
 * @return Message received from monitor
 * @attention Message provided is produced by the method. You must delete it when you are done using it
 * @warning Read is not thread safe : check that multiple tasks can't access this method simultaneously  
 */
Message *ComMonitor::Read() {
    char length = 0;
    string s;
    char data;
    bool endReception = false;
    Message *msg;

    // Call user method before read
    Read_Pre();

    if (clientID > 0) {
        while (!endReception) {
            if ((length = recv(clientID, (void*) &data, 1, MSG_WAITALL)) > 0) {
                if (data != '\n') {
                    s += data;
                } else endReception = true;
            }
        }

        if (length <= 0) msg = new Message(MESSAGE_MONITOR_LOST);
        else {
            msg = StringToMessage(s);
        }
    }

    // Call user method after read
    Read_Post();

    return msg;
}

/**
 * Method used internally to convert a message content to a string that can be sent over TCP
 * @param msg Message to be converted
 * @return A string, image of the message
 */
string ComMonitor::MessageToString(Message *msg) {
    int id;
    string str;
    //Message *localMsg = msg;
    Position pos;

    Img *image;
    Jpg jpeg ;
    string s;
                    
    id = msg->GetID();

    switch (id) {
        case MESSAGE_ANSWER_ACK :
            str.append(LABEL_MONITOR_ANSWER_ACK);
            break;
        case MESSAGE_ANSWER_NACK:
            str.append(LABEL_MONITOR_ANSWER_NACK);
            break;
        case MESSAGE_ANSWER_ROBOT_TIMEOUT:
            str.append(LABEL_MONITOR_ANSWER_TIMEOUT);
            break;
        case MESSAGE_ANSWER_ROBOT_UNKNOWN_COMMAND:
            str.append(LABEL_MONITOR_ANSWER_CMD_REJECTED);
            break;
        case MESSAGE_ANSWER_ROBOT_ERROR:
            str.append(LABEL_MONITOR_ANSWER_CMD_REJECTED);
            break;
        case MESSAGE_ANSWER_COM_ERROR:
            str.append(LABEL_MONITOR_ANSWER_COM_ERROR);
            break;
        case MESSAGE_CAM_POSITION:
            pos = ((MessagePosition*) msg)->GetPosition();

            str.append(LABEL_MONITOR_CAMERA_POSITION + LABEL_SEPARATOR_CHAR + to_string(pos.robotId) + ";" +
                    to_string(pos.angle) + ";" + to_string(pos.center.x) + ";" + to_string(pos.center.y) + ";" +
                    to_string(pos.direction.x) + ";" + to_string(pos.direction.y));
            break;
        case MESSAGE_CAM_IMAGE:
            image=((MessageImg*) msg)->GetImage();
            jpeg = image->ToJpg();
            
            //cout << "Jpeg size: " << to_string(jpeg.size())<<endl<<flush;
            
            s = base64_encode(jpeg.data(), jpeg.size());
            str.append(LABEL_MONITOR_CAMERA_IMAGE + LABEL_SEPARATOR_CHAR + s);
            break;
        case MESSAGE_ROBOT_BATTERY_LEVEL:
            str.append(LABEL_MONITOR_ROBOT_BATTERY_LEVEL + LABEL_SEPARATOR_CHAR + to_string(((MessageBattery*) msg)->GetLevel()));
            break;
        case MESSAGE_ROBOT_STATE_BUSY:
            str.append(LABEL_MONITOR_ROBOT_CURRENT_STATE + LABEL_SEPARATOR_CHAR + "1");
            break;
        case MESSAGE_ROBOT_STATE_NOT_BUSY:
            str.append(LABEL_MONITOR_ROBOT_CURRENT_STATE + LABEL_SEPARATOR_CHAR + "0");
            break;
        case MESSAGE_LOG:
            str.append(LABEL_MONITOR_MESSAGE + LABEL_SEPARATOR_CHAR + ((MessageString*) msg)->GetString());
            break;
        case MESSAGE_EMPTY:
            str.append(""); //empty string
            break;
        default:
            cerr<<"["<<__PRETTY_FUNCTION__<<"] (from ComMonitor::Write): Invalid message to send ("<<msg->ToString()<<")"<<endl<<flush;
            throw std::runtime_error {"Invalid message to send"};
    }

    str.append("\n");

    return str;
}

/**
 * Method used internally to convert a string received over TCP to a message 
 * @param s String containing message
 * @return A message, image of the string
 */
Message *ComMonitor::StringToMessage(string &s) {
    Message *msg;
    size_t pos;
    string org = s;
    string tokenCmd;
    string tokenData;

    /* Separate command from data if string contains a ':' */
    if ((pos = org.find(LABEL_SEPARATOR_CHAR)) != string::npos) {
        tokenCmd = org.substr(0, pos);
        org.erase(0, pos + 1);
        tokenData = org;
    } else tokenCmd = org;

    /* Convert command to message */
    if (tokenCmd.find(LABEL_MONITOR_ROBOT_MOVE) != string::npos) {
        msg = new MessageInt(MESSAGE_ROBOT_MOVE, stoi(tokenData));
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_TURN) != string::npos) {
        msg = new MessageInt(MESSAGE_ROBOT_TURN, stoi(tokenData));
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_START_WITHOUT_WD) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_START_WITHOUT_WD);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_START_WITH_WD) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_START_WITH_WD);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_RELOAD_WD) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_RELOAD_WD);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_PING) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_PING);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_RESET) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_RESET);
    } else if (tokenCmd.find(LABEL_MONITOR_CAMERA_ARENA_ASK) != string::npos) {
        msg = new Message(MESSAGE_CAM_ASK_ARENA);
    } else if (tokenCmd.find(LABEL_MONITOR_CAMERA_ARENA_CONFIRM) != string::npos) {
        msg = new Message(MESSAGE_CAM_ARENA_CONFIRM);
    } else if (tokenCmd.find(LABEL_MONITOR_CAMERA_ARENA_INFIRM) != string::npos) {
        msg = new Message(MESSAGE_CAM_ARENA_INFIRM);
    } else if (tokenCmd.find(LABEL_MONITOR_CAMERA_CLOSE) != string::npos) {
        msg = new Message(MESSAGE_CAM_CLOSE);
    } else if (tokenCmd.find(LABEL_MONITOR_CAMERA_OPEN) != string::npos) {
        msg = new Message(MESSAGE_CAM_OPEN);
    } else if (tokenCmd.find(LABEL_MONITOR_CAMERA_POSITION_COMPUTE) != string::npos) {
        msg = new Message(MESSAGE_CAM_POSITION_COMPUTE_START);
    } else if (tokenCmd.find(LABEL_MONITOR_CAMERA_POSITION_STOP) != string::npos) {
        msg = new Message(MESSAGE_CAM_POSITION_COMPUTE_STOP);
    } else if (tokenCmd.find(LABEL_MONITOR_MESSAGE) != string::npos) {
        msg = new MessageString(MESSAGE_LOG, tokenData);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_COM_CLOSE) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_COM_CLOSE);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_COM_OPEN) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_COM_OPEN);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_GET_BATTERY) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_BATTERY_GET);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_GET_STATE) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_STATE_GET);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_GO_FORWARD) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_GO_FORWARD);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_GO_BACKWARD) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_GO_BACKWARD);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_GO_LEFT) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_GO_LEFT);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_GO_RIGHT) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_GO_RIGHT);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_STOP) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_STOP);
    } else if (tokenCmd.find(LABEL_MONITOR_ROBOT_POWEROFF) != string::npos) {
        msg = new Message(MESSAGE_ROBOT_POWEROFF);
    } else {
        msg = new Message(MESSAGE_EMPTY);
    }

    return msg;
}
