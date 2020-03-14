#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <iostream>
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

#define PORT 6699

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof (address);
    char buffer[1024] = {0};

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
    cout << ">>> waiting the opening" << endl;
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
            (socklen_t*) & addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    cout << ">>> The robot is ready to receive something" << endl;
    unsigned long starttime = (unsigned long) time(NULL);
    while (1) {
        valread = read(new_socket, buffer, 1024);
        if (valread <= 0)
            break;
        printf(">>> I received : %s\n", buffer);
        cout << (unsigned long) time(NULL) << ": ";
        string s = "";
        s += LABEL_ROBOT_OK;
        switch (buffer[0]) {
            case LABEL_ROBOT_START_WITHOUT_WD:
                s += LABEL_ROBOT_OK;
                break;
            case LABEL_ROBOT_START_WITH_WD:
                s += LABEL_ROBOT_OK;
                break;
            case LABEL_ROBOT_MOVE:
                switch (buffer[2]) {
                    case '0':
                        cout << "oo I stop oo" << endl;
                        break;
                    case '-':
                        cout << "\\/ I move backward \\/" << endl;
                        break;
                    default:
                        cout << "/\\ I move forward /\\" << endl;
                        break;
                }
                s += LABEL_ROBOT_OK;
                break;
            case LABEL_ROBOT_TURN:
                switch (buffer[2]) {
                    case '-':
                        cout << "<< I turn to the left <<" << endl;
                        break;
                    default:
                        cout << ">> I turn to the right >>" << endl;
                        break;
                }
                s += LABEL_ROBOT_OK;
                break;
            case LABEL_ROBOT_GET_BATTERY:
                if ((unsigned long) time(NULL) - starttime > 20) {
                    s += '0';
                } else {
                    if ((unsigned long) time(NULL) - starttime > 10) {
                        s += '1';
                    } else {
                        s += '2';
                    }
                }



                break;
            case '0':
                //msg = new MessageBattery(MESSAGE_ROBOT_BATTERY_LEVEL, BATTERY_EMPTY);
                break;
            case '1':
                //msg = new MessageBattery(MESSAGE_ROBOT_BATTERY_LEVEL, BATTERY_LOW);
                break;
            case '2':
                //msg = new MessageBattery(MESSAGE_ROBOT_BATTERY_LEVEL, BATTERY_FULL);
                break;
            default:
                //msg = new Message(MESSAGE_ANSWER_ROBOT_ERROR);
                cerr << "[" << __PRETTY_FUNCTION__ << "] Unknown message received from robot (" << buffer << ")" << endl << flush;
        }

        send(new_socket, s.c_str(), s.length(), 0);
        //printf("%s sent\n", s);
    }
    cout << "The robot is dead. End of story " << endl;
    cout << " /\\_/\\" << endl << "( o.o )" << endl << " > ^ <" << endl;
    return 0;
} 