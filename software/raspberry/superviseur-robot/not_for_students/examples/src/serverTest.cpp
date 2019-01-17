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

#include <iostream>
#include "Robot.h"
#include "TcpServer.h"

int main (void)
{
    TcpServer server;
    int clientFD;
    string msgIn, msgOut,tmp;
    bool finish;

    Robot robot;

    cout << "TCP server example" << endl;

    cout << "Bind and listen on port 1337: ";
    try {
        server.Listen(1337);
    }
    catch ( const invalid_argument &ia ) {
        cerr << "Error binding server: " << ia.what() << endl;
        return 1;
    }

    cout << "Ok" <<endl;

    cout << "Open com with robot: ";
    try {
        robot.Open ("/dev/ttyUSB0");
    } catch (string e)
    {
        cerr << e << endl;
        return 3;
    }

    cout << "Ok" <<endl;

    while (1)
    {
        cout << "Wait for client to connect: ";
        try {
            clientFD= server.AcceptClient();
        }
        catch ( const invalid_argument &ia ) {
            cerr << "Error during client accept: " << ia.what() << endl;
            return 2;
        }

        cout << to_string(clientFD) << endl;

        msgIn.clear();
        finish=false;

        do {
            msgIn = server.Receive(clientFD, 2); // cmd + \n

            if (msgIn.empty()) finish=true;
            else {
                try {
                    switch (msgIn[0]) {
                        case 's':
                            // Start robot
                            robot.StartWithoutWatchdog();
                            break;
                        case 'f':
                            robot.Move(100);
                            break;
                        case 'b':
                            robot.Move(-100);
                            break;
                        case 'r':
                            robot.Turn(90);
                            break;
                        case 'l':
                            robot.Turn(-90);
                            break;
                        case 'Q':
                            robot.Reset();
                            break;
                    }
                } catch (string e) {
                    cerr << e;
                }

                if (robot.GetLastCommandStatus() == SUCCESS)
                    server.Send(clientFD, "OK\n");
                else server.Send(clientFD, "ERR\n");
            }
        } while (finish==false);

        cout << "Client disconnected" <<endl;
        clientFD=-1;
    }

    return 0;
}
