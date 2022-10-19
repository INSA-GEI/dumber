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

#ifndef __COMROBOT_H__
#define __COMROBOT_H__

#include "messages.h"
#include <string>

using namespace std;

/**
 * Class used for communicating with robot over serial
 * 
 * @brief Communication class with robot
 * 
 */
class ComRobot {
public:

    /**
     * Constructor
     */
    ComRobot() {
    }

    /**
     * Destructor
     */
    virtual ~ComRobot() {
    }

    /**
     * Open serial link with robot
     * @return File descriptor
     * @throw std::runtime_error if it fails
     */
    int Open();

    /**
     * Open serial link with robot
     * @param usart Filename of usart to open
     * @return File descriptor
     * @throw std::runtime_error if it fails
     */
    int Open(string usart);

    /**
     * Close serial link
     * @return Success if above 0, failure if below 0
     */
    int Close();

    /**
     * Send a message to robot
     * @param msg Message to send to robot
     * @return A message containing either an answer (Ack/Nak/Timeout/Error) or a value (battery level, robot state) depending of the command
     * @attention Input message is destroyed (delete) after being sent. You do not need to delete it yourself
     * @attention Write produce an answer message. You have to dispose it (delete) when you have finished using it
     * @attention Write is blocking until message is written into buffer (linux side)
     * @warning Write is not thread save : check that multiple tasks can't access this method simultaneously  
     */
    Message *Write(Message* msg);

    /**
     * Function called at beginning of Write method
     * Use it to do some synchronization (call of mutex, for example)
     */
    virtual void Write_Pre() {
    }

    /**
     * Function called at end of Write method
     * Use it to do some synchronization (call of mutex, for example)
     */
    virtual void Write_Post() {
    }
    
    Message *SendCommand(Message* msg, MessageID answerID, int maxRetries);
    
    static Message *Ping() {
        return new Message(MESSAGE_ROBOT_PING);
    }

    static Message *Reset() {
        return new Message(MESSAGE_ROBOT_RESET);
    }

    static Message *PowerOff() {
        return new Message(MESSAGE_ROBOT_POWEROFF);
    }

    static Message *StartWithWD() {
        return new Message(MESSAGE_ROBOT_START_WITH_WD);
    }

    static Message *StartWithoutWD() {
        return new Message(MESSAGE_ROBOT_START_WITHOUT_WD);
    }

    static Message *ReloadWD() {
        return new Message(MESSAGE_ROBOT_RELOAD_WD);
    }

    static Message *Move(int length) {
        return new MessageInt(MESSAGE_ROBOT_MOVE, length);
    }

    static Message *Turn(int angle) {
        return new MessageInt(MESSAGE_ROBOT_TURN, angle);
    }

    static Message *Stop() {
        return new Message(MESSAGE_ROBOT_STOP);
    }

    static Message *GoForward() {
        return new Message(MESSAGE_ROBOT_GO_FORWARD);
    }

    static Message *GoBackward() {
        return new Message(MESSAGE_ROBOT_GO_BACKWARD);
    }

    static Message *GoLeft() {
        return new Message(MESSAGE_ROBOT_GO_LEFT);
    }

    static Message *GoRight() {
        return new Message(MESSAGE_ROBOT_GO_RIGHT);
    }

    static Message *GetBattery() {
        return new Message(MESSAGE_ROBOT_BATTERY_GET);
    }

    static Message *GetState() {
        return new Message(MESSAGE_ROBOT_STATE_GET);
    }

protected:
    /**
     * Serial link file descriptor
     */
    int fd;

    /**
     * Get an answer from robot
     * @return String containing answer from robot
     * @attention Read method is blocking until a message is received (timeout of 500 ms)
     * @warning Read is not thread safe : Do not call it in multiple tasks simultaneously
     */
    string Read();

    /**
     * Convert a string to its message representation (when receiving data from robot)
     * @param s String from robot containing answer
     * @return Message corresponding to received array of char
     */
    Message* StringToMessage(string s);

    /**
     * Convert a message to its string representation (for sending command to robot)
     * @param msg Message to be sent to robot
     * @return String containing command to robot
     */
    string MessageToString(Message *msg);

    /**
     * Add a checksum and carriage return to a command string
     * @param[in,out] s String containing command for robot, without ending char (carriage return) 
     */
    void AddChecksum(string &s);

    /**
     * Verify if checksum of an incoming answer from robot is valid, 
     * then remove checksum from incoming answer (if checksum is ok)
     * @param[in,out] s String containing incoming answer from robot
     * @return true is checksum is valid, false otherwise.
     */
    bool VerifyChecksum(string &s);
};

#endif /* __COMROBOT_H__ */
