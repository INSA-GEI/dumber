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
struct sockaddr_in address;
int addrlen;
#define PORT 6699

int status = 0;
int noerr = 0;
int isWD = 0;

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
        printf("[I don't understand what you said (-1)]\n");
        return -1;
    } else if (r > 900) {
        printf("[I'm mute, because I never got your message (-2)]\n");
        return -2;
    }
    printf("[WILCO (0)] ");
    return 0;
}

void simulate_transmission_time() {
    usleep((rand() % 30) * 1000);
}

void open_server() {

    int opt = 1;
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

    cout << "<<< simulator >>>" << endl;    
    cout << ">>> Hello, I'm Mr " ;
    if (noerr) cout << "perfect ";
    cout << "Robot" << endl;

    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *) &address,
            sizeof (address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    cout << ">>> I create a server" << endl;
    cout << ">>> ..." << endl;

}

void wait_connection() {
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    cout << ">>> I'm waiting a client" << endl;
    if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
            (socklen_t*) & addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}

void reset(){
    isWD = 0;
    status = 0;   
    cout << ">>> XX I stop XX" << endl;
}

int main(int argc, char const *argv[]) {
    if (argc != 1){
        if (argv[1] == std::string("noerror")){
            noerr = 1;
        }
    }

    char buffer[1024] = {0};
    addrlen = sizeof (address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    srand(time(NULL));

    open_server();
    wait_connection();
    cout << ">>> I'm ready to receive something" << endl;
    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    struct timespec start_wd;


    struct timespec t;

    long int e;
    struct timespec last_call;
    clock_gettime(CLOCK_REALTIME, &last_call);
    isWD = 0;

    while (status < 3) {

        if (isWD) {
            clock_gettime(CLOCK_REALTIME, &t);
            e = ellapse(last_call, t);
            if ((e / 1000000000) > 3) {
                cout << ">>> You break my heart, you never talk at the right time." << endl;
                break;
            }
        }


        valread = read(new_socket, buffer, 1024);
        if (valread <= 0) {
            if (errno == EAGAIN) {
                status = 3;
                cout << ">>> You break my heart, I've been waiting too long for you." << endl;
                break;
            } else {
                cout << ">>> Why did you hang up? Please, contact me again." << endl;
                reset();
                wait_connection();
                clock_gettime(CLOCK_REALTIME, &last_call);
            }
        }
        string s = "";
        int error = 0;
        
        if (!noerr) error = simulate_error();
        
        if (error == 0) {

            print_time(start_time);
            printf(": I received a message %s\n", buffer);
            switch (buffer[0]) {
                case LABEL_ROBOT_START_WITHOUT_WD:
                    cout << ">>> I start without watchdog" << endl;
                    s += LABEL_ROBOT_OK;
                    break;
                case LABEL_ROBOT_PING:
                    cout << ">>> ...Pong" << endl;
                    s += LABEL_ROBOT_OK;
                    break;
                case LABEL_ROBOT_START_WITH_WD:
                    clock_gettime(CLOCK_REALTIME, &start_wd);
                    clock_gettime(CLOCK_REALTIME, &last_call);
                    struct timeval tv;
                    tv.tv_sec = 3;
                    tv.tv_usec = 0;
                    setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof tv);

                    cout << ">>> I start with watchdog" << endl;
                    s += LABEL_ROBOT_OK;
                    isWD = 1;
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
                    clock_gettime(CLOCK_REALTIME, &t);
                    e = ellapse(start_wd, t);
                    e = (e / 1000000) % 1000;
                    if (isWD) {
                        if ((e < 50) || (e > 950)) {
                            cout << ">>> Just in time for a reload " << e << "ms" << endl;
                            last_call = t;
                            status = 0;
                            s += LABEL_ROBOT_OK;
                        } else {
                            status++;
                            cout << ">>> You missed the date, -1 point " << e << "ms (" << status << ")" << endl;

                            s += LABEL_ROBOT_UNKNOWN_COMMAND;
                        }
                    } else {
                        cout << "Why you said that, I do nothing" << endl;
                    }
                    break;
                case LABEL_ROBOT_POWEROFF:
                    cout << ">>> Bye bye, see you soon" << endl;
                    s += LABEL_ROBOT_OK;
                    status = 10;
                    break;
                case LABEL_ROBOT_RESET:
                    cout << ">>> I reset" << endl;
                    s += LABEL_ROBOT_OK;
                    reset();
                    break;
                case LABEL_ROBOT_GET_STATE:
                    cout << ">>> I'm fine, thank you" << endl;
                    s += LABEL_ROBOT_OK;
                    break;
                default:
                    //msg = new Message(MESSAGE_ANSWER_ROBOT_ERROR);
                    cerr << "[" << __PRETTY_FUNCTION__ << "] Unknown message received from robot (" << buffer << ")" << endl << flush;
            }
            simulate_transmission_time();
            send(new_socket, s.c_str(), s.length(), 0);
        } else if (error == -1) {
            s += LABEL_ROBOT_UNKNOWN_COMMAND;
            simulate_transmission_time();
            send(new_socket, s.c_str(), s.length(), 0);
        } else if (error == -2) {
            /* Do nothing */
        }
    }

    cout << "The robot is out. End of story. " << endl;
    cout << " /\\_/\\" << endl << "( o.o )" << endl << " > ^ <" << endl;
    return 0;
} 