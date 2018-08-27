#include <iostream>
#include "../../src/definitions.h"
#include "../../src/robot.h"
#include <ctime>

#define NB_TEST 50
#define NB_RETRIES_MAX 5
#define DELAY_BETWEEN_RETRIES 10000 // 10 ms
#define DELAY_BETWEEN_TEST 1000000 // 1 seconde

int nb_retries;
int nb_test;

int laststatus;
int nb_success;
int nb_timeout;
int nb_unknown_cmd;

int flipflop;

int main() {
    if (open_communication_robot("/dev/ttyUSB0") != 0) {
        std::cout << "Unable to open com port" << std::endl;

        return -1;
    }

    std::cout << "Debut du test de stress" << std::endl;
    time_t t_debut, t_fin;
    struct tm *tm_debut, *tm_fin;

    time(&t_debut);
    tm_debut = (struct tm*)malloc (sizeof(struct tm));
    tm_debut = localtime((const time_t*)&t_debut);

    std::cout << "[heure debut]: " << asctime(tm_debut) << std::endl;

    nb_retries=-1;
    do {
        usleep(DELAY_BETWEEN_RETRIES);
        nb_retries++;
        laststatus = send_command_to_robot(DMB_START_WITHOUT_WD);
    } while ((nb_retries<NB_RETRIES_MAX) && (laststatus != 0));

    if(nb_retries == NB_RETRIES_MAX) {
        std::cout << "Unable to start robot" <<std::endl;

        return -1;
    }

    nb_success =0;
    nb_timeout =0;
    nb_unknown_cmd =0;
    flipflop = 0;

    for (nb_test=0; nb_test < NB_TEST; nb_test++) {
        nb_retries=-1;

        do {
            usleep(DELAY_BETWEEN_RETRIES);
            nb_retries++;
            if (flipflop == 0) {
                laststatus = send_command_to_robot(DMB_TURN, "100");
            } else {
                laststatus = send_command_to_robot(DMB_TURN, "-100");
            }

        } while ((nb_retries<NB_RETRIES_MAX) && (laststatus != 0));

        if (flipflop ==0) flipflop=1;
        else flipflop =0;

        if (laststatus == 0) nb_success++;
        else if (laststatus == ROBOT_TIMED_OUT) nb_timeout++;
        else if (laststatus == ROBOT_UKNOWN_CMD) nb_unknown_cmd ++;

        std::cout << "Test " << nb_test << " [s " << nb_success
                  << ": t " << nb_timeout << ": u " << nb_unknown_cmd
                  << "]"<< "\x1B" <<"[30D"<<std::flush;

        usleep(DELAY_BETWEEN_TEST);
    }

    time(&t_fin);
    tm_fin = (struct tm*)malloc (sizeof(struct tm));
    tm_fin = localtime((const time_t*)&t_fin);

    std::cout << std::endl << std::endl << "[heure fin]: " << asctime(tm_fin) << std::endl;

    return 0;
}