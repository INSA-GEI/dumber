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
#include <ctime>
#include "Robot.h"
#include <unistd.h>

using namespace std;

#define NB_TEST 50
#define DELAY_BETWEEN_TEST 1000000 // 1 seconde

int nb_retries;
int nb_test;

int laststatus;
int nb_success;
int nb_timeout;
int nb_unknown_cmd;

int flipflop;

int Test(Robot rob) {
    try {
        if (flipflop == 0) {
            rob.Move(100);
        } else {
            rob.Move(-100);
        }
    } catch (string e) {
        //if (e.find("Timeout")==string.npos)
        //    status=ROBOT_TIMED_OUT;


    }

    return rob.GetLastCommandStatus();;
}

int main() {
    Robot myRobot;

    if (myRobot.Open("/dev/ttyUSB0") == SUCCESS) { // ouverture de la com avec le robot

        std::cout << "Start robot: ";
        try {
            myRobot.StartWithoutWatchdog();
        } catch ( string e ) {
            std::cerr << std::endl << e << std::endl;
            return 1;
        }

        if (myRobot.GetLastCommandStatus()==SUCCESS)
            std::cout << "Ok" <<std::endl;
        else {
            std::cout << " Error: GetLastCommand returns " << myRobot.GetLastCommandStatus() <<std::endl;
            return 2;
        }

        std::cout << "Start stress test" << std::endl;
        time_t t_debut, t_fin;
        struct tm *tm_debut, *tm_fin;

        time(&t_debut);
        tm_debut = localtime((const time_t*)&t_debut);
        std::cout << "[start time]: " << asctime(tm_debut) << std::endl;

        nb_success =0;
        nb_timeout =0;
        nb_unknown_cmd =0;
        flipflop = 0;

        for (nb_test=0; nb_test < NB_TEST; nb_test++) {
            laststatus=Test(myRobot);

            if (flipflop ==0) flipflop=1;
            else flipflop =0;

            if (laststatus == SUCCESS) nb_success++;
            else if (laststatus == TIMEOUT_COMMAND) nb_timeout++;
            else if ((laststatus == INVALID_ANSWER) || (laststatus == INVALID_COMMAND)) nb_unknown_cmd ++;

            std::cout << "Test " << nb_test << " [s " << nb_success
                    << ": t " << nb_timeout << ": u " << nb_unknown_cmd
                    << "]"<< "\x1B" <<"[30D"<<std::flush;

            usleep(DELAY_BETWEEN_TEST);
        }

        time(&t_fin);
        tm_fin = localtime((const time_t*)&t_fin);
        std::cout << std::endl << std::endl << "[end time]: " << asctime(tm_fin) << std::endl;

        std::cout << "Reset robot" <<std::endl;
        myRobot.Reset();

        std::cout << "Close com" <<std::endl;
        myRobot.Close();

    } else {
        std::cerr << "Unable to open \\dev\\ttyUSB0" << std::endl;

        return 2;
    }

    //    if (open_communication_robot("/dev/ttyUSB0") != 0) {
    //        std::cout << "Unable to open com port" << std::endl;
    //
    //        return -1;
    //    }
    //
    //    std::cout << "Debut du test de stress" << std::endl;
    //    time_t t_debut, t_fin;
    //    struct tm *tm_debut, *tm_fin;
    //
    //    time(&t_debut);
    //    tm_debut = localtime((const time_t*)&t_debut);
    //
    //    std::cout << "[heure debut]: " << asctime(tm_debut) << std::endl;
    //
    //    nb_retries=-1;
    //    do {
    //        usleep(DELAY_BETWEEN_RETRIES);
    //        nb_retries++;
    //        laststatus = send_command_to_robot(DMB_START_WITHOUT_WD);
    //    } while ((nb_retries<NB_RETRIES_MAX) && (laststatus != 0));
    //
    //    if(nb_retries == NB_RETRIES_MAX) {
    //        std::cout << "Unable to start robot" <<std::endl;
    //
    //        return -1;
    //    }
    //
    //    nb_success =0;
    //    nb_timeout =0;
    //    nb_unknown_cmd =0;
    //    flipflop = 0;
    //
    //    for (nb_test=0; nb_test < NB_TEST; nb_test++) {
    //        nb_retries=-1;
    //
    //        do {
    //            usleep(DELAY_BETWEEN_RETRIES);
    //            nb_retries++;
    //            if (flipflop == 0) {
    //                laststatus = send_command_to_robot(DMB_TURN, "100");
    //            } else {
    //                laststatus = send_command_to_robot(DMB_TURN, "-100");
    //            }
    //
    //        } while ((nb_retries<NB_RETRIES_MAX) && (laststatus != 0));
    //
    //        if (flipflop ==0) flipflop=1;
    //        else flipflop =0;
    //
    //        if (laststatus == 0) nb_success++;
    //        else if (laststatus == ROBOT_TIMED_OUT) nb_timeout++;
    //        else if (laststatus == ROBOT_UKNOWN_CMD) nb_unknown_cmd ++;
    //
    //        std::cout << "Test " << nb_test << " [s " << nb_success
    //                  << ": t " << nb_timeout << ": u " << nb_unknown_cmd
    //                  << "]"<< "\x1B" <<"[30D"<<std::flush;
    //
    //        usleep(DELAY_BETWEEN_TEST);
    //    }
    //
    //    time(&t_fin);
    //    tm_fin = localtime((const time_t*)&t_fin);
    //
    //    std::cout << std::endl << std::endl << "[heure fin]: " << asctime(tm_fin) << std::endl;

    return 0;
}
