/*******************************************************************************
 * Copyright (c) 2018 INSA - GEI, Toulouse, France.
 * All rights reserved. This program and the accompanying materials
 * are made available "AS IS", without any warranty of any kind.
 *
 * INSA assumes no responsibility for errors or omissions in the 
 * software or documentation available. 
 *
 * Part of code Copyright ST Microelectronics.
 *
 * Contributors:
 *     Lucien Senaneuch - Initial API and implementation
 *     Sebastien DI MERCURIO - Maintainer since Octobre 2018
 *******************************************************************************/

#include <Serial.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <stdlib.h>

#include <algorithm>

const string Serial::TimeoutException = "Timeout exception";
const string Serial::IOErrorException = "IO error exception";
const string Serial::BufferOverflowException = "Buffer overflow exception";

Serial::Serial() {
    this->serialPortDescriptor=-1;
}

Serial::~Serial() {
    this->Close();
}

char Serial::Open(void) {
    return this->Open(DEFAULT_SERIAL_PORT, DEFAULT_BAUDRATE);
}

char Serial::Open(int baudrate) {
    return this->Open(DEFAULT_SERIAL_PORT, baudrate);
}

char Serial::Open(const char *path, int baudrate) {
#ifndef __STUB__
    struct termios options;

    if (this->IsOpen() == true) return COMMUNICATION_PORT_ALREADY_OPENED;

    this->serialPortDescriptor = open(path, O_RDWR | O_NOCTTY | O_NDELAY);

    if(this->serialPortDescriptor >= 0) {
        fcntl(this->serialPortDescriptor, F_SETFL, 0);
        tcgetattr(this->serialPortDescriptor, &options);
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        cfsetospeed (&options, this->SetBaudrate(baudrate));
        cfsetispeed (&options, this->SetBaudrate(baudrate));
        options.c_cc[VMIN]=0;
        options.c_cc[VTIME]=10;
        tcsetattr(this->serialPortDescriptor, TCSANOW, &options);

        return SUCCESS;
    }
    else {
        std::cerr << "[Robot::Open] Can't open port " << path;
        return INVALID_COMMUNICATION_PORT;
    }
#else
    return SUCCESS;
#endif
}

char Serial::Close(void) {
#ifndef __STUB__
    if (this->serialPortDescriptor<0) {
        if (close(this->serialPortDescriptor) == 0)
            return SUCCESS;
        else return INVALID_COMMUNICATION_PORT;
    } else return SUCCESS;
#else
    return SUCCESS;
#endif
}

bool Serial::IsOpen() {
    if(this->serialPortDescriptor >= 0)
        return true;
    else
        return false;
}

ssize_t Serial::Send(string mes) {
    ssize_t n;

    n=write(this->serialPortDescriptor, (void*)mes.c_str(), mes.length());

    if (n< (ssize_t)mes.length())
        throw (IOErrorException + " raised in Serial::Send. Requested to send " + to_string(mes.length()) + " characters, sent only " + to_string(n) +"\n");
    else
        return n;
}

string Serial::Receive(int size) {
    string mes;

    while(mes.length() < (unsigned int)size){
        mes += this->GetChar();
    }

    return mes;
}

string Serial::Receive(vector<char> endingChars,int maxLength) {
    string mes;
    char c;

    bool characterFound = false;

    do {
        c= GetChar();

        if (std::find(endingChars.begin(), endingChars.end(), c)!=endingChars.end()) //recherche du caractere dans la liste des caractere de fin
            characterFound=true;
        else {
            mes += c;
        }
    } while ((mes.length()<(unsigned int)maxLength) && (characterFound==false));

    if (mes.length()>=(unsigned int)maxLength) {
        throw (BufferOverflowException + " raised in Serial::Receive. Received data exceed " + to_string(maxLength) + "chars\n");
    }

    return mes;
}

int Serial::SetBaudrate(int baudrate) {
    switch(baudrate){
        case 4800:
            return baudrate = B4800;
        case 9600:
            return baudrate = B9600;
        case 38400:
            return baudrate = B38400;
        case 57600:
            return baudrate = B57600;
        case 115200:
            return baudrate = B115200;
        default:
            std::cerr << "Baudrate not supported" << std::endl;
            return -1;
    }
}

char Serial::GetChar() {
    char c;

    if (read(this->serialPortDescriptor, &c, 1) > 0)
        return c;
    else
        throw (TimeoutException + " raised in Serial::GetChar\n");
}


