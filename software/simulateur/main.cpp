#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <iostream>
#include <time.h>
using namespace std;

const char LABEL_ROBOT_PING = 'p';
const char LABEL_ROBOT_RESET = 'r';
const char LABEL_ROBOT_START_WITH_WD = 'W';
const char LABEL_ROBOT_START_WITHOUT_WD = 'u';
const char LABEL_ROBOT_RELOAD_WD = 'w';
const char LABEL_ROBOT_MOVE = 'M';
const char LABEL_ROBOT_TURN = 'T';
const char LABEL_ROBOT_GET_BATTERY = 'v';
const char LABEL_ROBOT_GET_STATE = 'b';
const char LABEL_ROBOT_POWEROFF = 'z';

const char LABEL_ROBOT_OK = 'O';
const char LABEL_ROBOT_ERROR = 'E';
const char LABEL_ROBOT_UNKNOWN_COMMAND = 'C';

const char LABEL_ROBOT_SEPARATOR_CHAR = '=';
const char LABEL_ROBOT_ENDING_CHAR = 0x0D;

int server_fd, new_socket, valread;
#define PORT 6699

long int ellapse(struct timespec ref, struct timespec cur) {
    long int e;
    e = cur.tv_sec - ref.tv_sec;
    e *= 1000000000;
    e += cur.tv_nsec - ref.tv_nsec;
    return e;
}

void print_time(struct timespec start_time) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    long int e = ellapse(start_time, t);
    fprintf(stdout, "%9ld", e / 1000000);
}

int simulate_error() {
    int r = rand() % 1000;
    if (r > 950) {
        printf(">>> I don't understand what you said (-1)\n");
        return -1;
    } else if (r > 900) {
        printf(">>> I don't want to respond (-2)\n");
        return -2;
    }
    printf(">>> WILCO (0)\n");
    return 0;
}

void simulate_transmission_time() {
    usleep((rand() % 30) * 1000);
}

void open_server() {
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof (address);

    srand(time(NULL));

    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
            &opt, sizeof (opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    cout << "<<< simulator >>>" << endl;

    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *) &address,
            sizeof (address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    cout << ">>> I create a server" << endl;
    cout << ">>> ..." << endl;
    cout << ">>> I'm waiting a client" << endl;
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    cout << ">>> Hello, I'm Robot" << endl;
    if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
            (socklen_t*) & addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[]) {

    char buffer[1024] = {0};
    open_server();
    cout << ">>> I'm ready to receive something" << endl;
    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    while (1) {
        valread = read(new_socket, buffer, 1024);
        if (valread <= 0)
            break;
        print_time(start_time);
        printf(" >>> I received a message : %s\n", buffer);
        string s = "";
        int error = simulate_error();
        if (error == 0) {
            struct timespec t;
            long int e;
            switch (buffer[0]) {
                case LABEL_ROBOT_START_WITHOUT_WD:
                    cout << ">>> I start without watchdog" << endl;
                    s += LABEL_ROBOT_OK;
                    break;
                case LABEL_ROBOT_START_WITH_WD:
                    s += LABEL_ROBOT_OK;
                    break;
                case LABEL_ROBOT_MOVE:
                    switch (buffer[2]) {
                        case '0':
                            cout << ">>> XX I stop XX" << endl;
                            break;
                        case '-':
                            cout << ">>> \\/ I move backward \\/" << endl;
                            break;
                        default:
                            cout << ">>> /\\ I move forward /\\" << endl;
                            break;
                    }
                    s += LABEL_ROBOT_OK;
                    break;
                case LABEL_ROBOT_TURN:
                    switch (buffer[2]) {
                        case '-':
                            cout << ">>> << I turn to the left <<" << endl;
                            break;
                        default:
                            cout << ">>> >> I turn to the right >>" << endl;
                            break;
                    }
                    s += LABEL_ROBOT_OK;
                    break;
                case LABEL_ROBOT_GET_BATTERY:
                    cout << ">>> I give you my battery level :-o" << endl;
                    clock_gettime(CLOCK_REALTIME, &t);
                    e = ellapse(start_time, t);
                    if (e > 20000000000) {
                        s += '0';
                    } else {
                        if (e > 10000000000) {
                            s += '1';
                        } else {
                            s += '2';
                        }
                    }
                    break;
                case LABEL_ROBOT_RELOAD_WD:
                    cout << ">>> I start with watchdog" << endl;
                    s += LABEL_ROBOT_OK;
                    break;
                case LABEL_ROBOT_POWEROFF:
                    cout << ">>> Bye bye" << endl;
                    s += LABEL_ROBOT_OK;
                    break;
                default:
                    //msg = new Message(MESSAGE_ANSWER_ROBOT_ERROR);
                    cerr << "[" << __PRETTY_FUNCTION__ << "] Unknown message received from robot (" << buffer << ")" << endl << flush;
            }
            simulate_transmission_time();
            send(new_socket, s.c_str(), s.length(), 0);
        } else if (error == -1){
            s += LABEL_ROBOT_UNKNOWN_COMMAND;
            simulate_transmission_time();
            send(new_socket, s.c_str(), s.length(), 0);
        } else if (error == -2){
            /* Do nothing */
        }
    }
    cout << "The robot is dead. End of story. " << endl;
    cout << " /\\_/\\" << endl << "( o.o )" << endl << " > ^ <" << endl;
    return 0;
} 