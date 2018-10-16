/*
 * Serial.h
 *
 *  Created on: 12 oct. 2018
 *      Author: dimercur
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define SUCCESS                             0
#define INVALID_COMMUNICATION_PORT          -1
#define COMMUNICATION_PORT_ALREADY_OPENED   -2

#define DEFAULT_SERIAL_PORT             "/dev/ttyS0"
#define DEFAULT_BAUDRATE                9600

class Serial {
    public:
        static const string TimeoutException;
        static const string IOErrorException;
        static const string BufferOverflowException;

        Serial();
        virtual ~Serial();

        char Open(void);
        char Open(int baudrate);
        char Open(const char *path, int baudrate);
        char Close(void);

        bool IsOpen();
        ssize_t Send(string mes);
        string Receive(int size);
        string Receive(vector<char> endingChars, int maxLength);

    private:
        int serialPortDescriptor;
        int SetBaudrate(int baudrate);
        char GetChar(); // read 1 char
};

#endif /* SERIAL_H_ */
