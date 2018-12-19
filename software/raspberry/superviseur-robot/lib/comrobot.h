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
    ComRobot() {}
    
    /**
     * Destructor
     */
    virtual ~ComRobot() {}
    
    /**
     * Open serial link with robot
     * @return File descriptor
     * @throw std::runtime_error if it fails
     */
    int Open();
    
    /**
     * Close serial link
     * @return Success if above 0, failure if below 0
     */
    int Close();
    
    /**
     * Get a message from robot
     * @return Message currently received
     * @attention A message object is created (new) when receiving data from robot. You MUST remember to destroy is (delete) after use
     * @attention Read method is blocking until a message is received
     * @warning Read is not thread safe : Do not call it in multiple tasks simultaneously
     */
    Message* Read();
    
    /**
     * Send a message to robot
     * @param msg Message to send to robot
     * @return 1 if success, 0 otherwise
     * @attention Message is destroyed (delete) after being sent. You do not need to delete it yourself
     * @attention Write is blocking until message is written into buffer (linux side)
     * @warning Write is not thread save : check that multiple tasks can't access this method simultaneously  
     */
    int Write(Message* msg);
    
    /**
     * Function called at beginning of Read method
     * Use it to do some synchronization (call of mutex, for example)
     */
    virtual void Read_Pre() {}
    
    /**
     * Function called at end of Read method
     * Use it to do some synchronization (call of mutex, for example)
     */
    virtual void Read_Post() {}
    
    /**
     * Function called at beginning of Write method
     * Use it to do some synchronization (call of mutex, for example)
     */
    virtual void Write_Pre() {}
    
    /**
     * Function called at end of Write method
     * Use it to do some synchronization (call of mutex, for example)
     */
    virtual void Write_Post() {}
    
    static Message *Ping();
    
protected:
    /**
     * Serial link file descriptor
     */
    int fd;
    
    /**
     * Convert an array of char to float
     * @param bytes Array of char, containing a binary image of a float
     * @return Float value
     */
    float CharToFloat(unsigned char *bytes);
    
    /**
     * Convert an array of char to boolean
     * @param bytes Array of char, containing a binary image of a boolean
     * @return Boolean value
     */
    bool CharToBool(unsigned char *bytes);
    
    /**
     * Convert an array of char to integer
     * @param bytes Array of char, containing a binary image of an integer
     * @return Integer value
     */
    unsigned int CharToInt(unsigned char *bytes);
    
    /**
     * Convert an array of char to its message representation (when receiving data from stm32)
     * @param bytes Array of char
     * @return Message corresponding to received array of char
     */
    Message* CharToMessage(unsigned char *bytes);
    
    /**
     * Convert a message to its array of char representation (for sending command to stm32)
     * @param msg Message to be sent to robot
     * @param buffer Array of char, image of message to send
     */
    void MessageToChar(Message *msg, unsigned char *buffer);
};

#endif /* __COMROBOT_H__ */
