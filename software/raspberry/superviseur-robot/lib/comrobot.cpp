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

#include "comrobot.h"

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>  

#include <string>
#include <stdexcept>

#ifdef __FOR_PC__
#define USART_FILENAME "/dev/ttyUSB0"
#else
#define USART_FILENAME "/dev/ttyS0"
#endif /* __FOR_PC__ */

/*
 * Constants to be used for communicating with robot. Contains command tag
 */
typedef enum {
    LABEL_ANGLE_POSITION = 'p',
    LABEL_ANGULAR_SPEED = 's',
    LABEL_BATTERY_LEVEL = 'b',
    LABEL_BETA_ANGLE = 'v',
    LABEL_USER_PRESENCE = 'u',

    LABEL_TORQUE = 'c',
    LABEL_EMERGENCY_STOP = 'a'
} LabelRobot;

/**
 * Open serial link with robot
 * @return File descriptor
 * @throw std::runtime_error if it fails
 */
int ComRobot::Open() {
    fd = open(USART_FILENAME, O_RDWR | O_NOCTTY /*| O_NDELAY*/); //Open in blocking read/write mode
    if (fd == -1) {
        //ERROR - CAN'T OPEN SERIAL PORT
        throw std::runtime_error{"Error - Unable to open UART " + string(USART_FILENAME) + ".  Ensure it is not in use by another application"};
        exit(EXIT_FAILURE);
    }

    //Configuration of the serial port 115 520 Bauds
    struct termios options;
    tcgetattr(fd, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD; //<Set baud rate
    options.c_iflag = IGNPAR; // ignores bytes with bad parity
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

/**
 * Close serial link
 * @return Success if above 0, failure if below 0
 */
int ComRobot::Close() {
    return close(fd);
}

/**
 * Get a message from robot
 * @return Message currently received
 * @attention A message object is created (new) when receiving data from robot. You MUST remember to destroy is (delete) after use
 * @attention Read method is blocking until a message is received
 * @warning Read is not thread safe : Do not call it in multiple tasks simultaneously
 */
Message* ComRobot::Read() {
    int rxLength;
    unsigned char rxBuffer[6];
    unsigned char receivedChar;
    bool messageComplete = false;
    Message *msg;
    unsigned int i;

    /* Call pre method for read */
    Read_Pre();

    /* a message is composed of 7 bytes.
                the byte 0 should always be '<'
                the byte 1 should be an ascii char that is the label. It define what the data represent
                the bytes 2 to 5 are the float value
                the byte 6 should always be a '\n'
     */
    while (messageComplete == false) {
        rxLength = read(this->fd, (void*) &receivedChar, 1); //Filestream, buffer to store in, number of bytes to read (max)
        //printf ("W=%02X ", receivedChar);

        if (rxLength <= -1) {
            this->lostCom = true;
            printf("Warning: communication lost in ComStm32::Read\n");
            msg = new Message();

            return msg;
        } else if (rxLength == 0) {
            // nothing to do
        } else if (receivedChar == '<') { // start of frame received
            i = 0;

            do {
                rxLength = read(this->fd, (void*) &rxBuffer[i], 6 - i); //Filestream, buffer to store in, number of bytes to read (max)

                if (rxLength >= 0)
                    i = i + rxLength;
                else {
                    printf("Error while reading (%i)", rxLength);

                    return NULL;
                }
            } while (i < 6);

            if (rxBuffer[5] == '\n') {
                messageComplete = true;
            }
        }
    }

    /* Treatment of received message */
    msg = CharToMessage(rxBuffer);

    /* Call Post method for read */
    Read_Post();

    return msg;
}

/**
 * Convert an array of char to its message representation (when receiving data from stm32)
 * @param bytes Array of char
 * @return Message corresponding to received array of char
 */
Message* ComRobot::CharToMessage(unsigned char *bytes) {
    Message *msg = __null;
    MessageFloat *msgf;
    MessageBool *msgb;

    switch (bytes[0]) {
        case LABEL_ANGLE_POSITION:
            msgf = new MessageFloat();
            msgf->SetID(MESSAGE_ANGLE_POSITION);
            msgf->SetValue(CharToFloat(&bytes[1]));
            msg = (Message*) msgf;

            break;
        case LABEL_ANGULAR_SPEED:
            msgf = new MessageFloat();
            msgf->SetID(MESSAGE_ANGULAR_SPEED);
            msgf->SetValue(CharToFloat(&bytes[1]));
            msg = (Message*) msgf;

            break;
        case LABEL_BATTERY_LEVEL:
            msgf = new MessageFloat();
            msgf->SetID(MESSAGE_BATTERY);
            msgf->SetValue(CharToFloat(&bytes[1]));
            msg = (Message*) msgf;

            break;
        case LABEL_BETA_ANGLE:
            msgf = new MessageFloat();
            msgf->SetID(MESSAGE_BETA);
            msgf->SetValue(CharToFloat(&bytes[1]));
            msg = (Message*) msgf;

            break;
        case LABEL_USER_PRESENCE:
            msgb = new MessageBool();
            msgb->SetID(MESSAGE_USER_PRESENCE);
            msgb->SetState(CharToBool(&bytes[1]));
            msg = (Message*) msgb;

            break;
        default:
            printf("Unknown message received from robot (%i)\n", bytes[0]);
            fflush(stdout);
            msg = new Message();
    }

    if (msg == NULL) {
        printf("Message is null (%02X)\n", bytes[0]);
        fflush(stdout);
        msg = new Message();
    }

    return msg;
}

/**
 * Convert an array of char to float
 * @param bytes Array of char, containing a binary image of a float
 * @return Float value
 */
float ComRobot::CharToFloat(unsigned char *bytes) {
    unsigned long value;

    union {
        unsigned char buffer[4];
        float f;
    } convert;

    convert.buffer[0] = bytes[0];
    convert.buffer[1] = bytes[1];
    convert.buffer[2] = bytes[2];
    convert.buffer[3] = bytes[3];

    //value = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | (bytes[0]);

    return convert.f;
}

/**
 * Convert an array of char to integer
 * @param bytes Array of char, containing a binary image of an integer
 * @return Integer value
 */
unsigned int ComRobot::CharToInt(unsigned char *bytes) {
    unsigned long value;

    value = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | (bytes[0]);

    return (unsigned int) value;
}

/**
 * Convert an array of char to boolean
 * @param bytes Array of char, containing a binary image of a boolean
 * @return Boolean value
 */
bool ComRobot::CharToBool(unsigned char *bytes) {
    unsigned long value;

    value = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | (bytes[0]);

    if (value == 0) return false;

    else return true;
}

/**
 * Send a message to robot
 * @param msg Message to send to robot
 * @return 1 if success, 0 otherwise
 * @attention Message is destroyed (delete) after being sent. You do not need to delete it yourself
 * @attention Write is blocking until message is written into buffer (linux side)
 * @warning Write is not thread save : check that multiple tasks can't access this method simultaneously  
 */
int ComRobot::Write(Message* msg) {
    unsigned char buffer[7];
    int ret_val = 0;

    MessageToChar(msg, buffer);

    Write_Pre();

    if (this->fd != -1) {
        int count = write(this->fd, &buffer[0], 7); //Filestream, bytes to write, number of bytes to write
        if (count < 0) {
            printf("Warning: UART TX error in ComStm32::Write\n");
        } else {
            ret_val = 1;
        }
    }

    // deallocation of msg
    delete(msg);

    Write_Post();

    return ret_val;
}

/**
 * Convert a message to its array of char representation (for sending command to stm32)
 * @param msg Message to be sent to robot
 * @param buffer Array of char, image of message to send
 */
void ComRobot::MessageToChar(Message *msg, unsigned char *buffer) {
    float val_f;
    int val_i;
    unsigned char *b;

    buffer[0] = '<';
    buffer[6] = '\n';

    switch (msg->GetID()) {
        case MESSAGE_TORQUE:
            buffer[1] = LABEL_TORQUE;
            val_f = ((MessageFloat*) msg)->GetValue();
            b = (unsigned char *) &val_f;

            break;
        case MESSAGE_EMERGENCY_STOP:
            buffer[1] = LABEL_EMERGENCY_STOP;
            if (((MessageBool*) msg)->GetState())
                val_i = 1;
            else
                val_i = 0;
            b = (unsigned char *) &val_i;

            break;
        default:
            printf("Invalid message to send");
            val_i = 0;
            b = (unsigned char *) &val_i;
    }

    buffer[2] = b[0];
    buffer[3] = b[1];
    buffer[4] = b[2];
    buffer[5] = b[3];
}

