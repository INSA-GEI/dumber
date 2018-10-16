#include "monitor.h"

using namespace std;

SOCKADDR_IN ssin;
SOCKET sock;
socklen_t recsize = sizeof (ssin);

/*Socket et contexte d'adressage du client */
SOCKADDR_IN csin;
SOCKET csock;
socklen_t crecsize = sizeof (csin);

int sock_err;
pid_t pidNodejs;

string serverReceive(int size);
int serverSend(const void *data, int dataLength);
int receive(char *data);

int run_nodejs(const char * path, char * file) {
    int ret;
    //char *const parmList[] = {"/bin/ls", "-l", "/home", NULL};
    char * const parmList[] = {"node", file, NULL};

    if ((ret = fork()) == -1)
        perror("fork error");
    else if (ret == 0) {
        ret = execv(path, parmList);
    }
    return ret;
}

int kill_nodejs() {
#ifdef __VERBOSE__  
    printf("kill nodejs\n");
#endif
    return kill(pidNodejs, SIGTERM);
}

int open_server(int port) {
    /* Création d'un socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    int enable = 1;

    /* Si la socket est valide */
    if (sock != INVALID_SOCKET) {
#ifdef _WITH_TRACE_
        printf("TCP/IP Socket %d opened\n", sock);
#endif
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (enable));

        /* Configuration */
        ssin.sin_addr.s_addr = htonl(INADDR_ANY); /* Adresse IP automatique */
        ssin.sin_family = AF_INET; /* Protocole familial (IP) */
        ssin.sin_port = htons(port); /* Listage du port */
        sock_err = bind(sock, (SOCKADDR*) & ssin, recsize);

        /* Si la socket fonctionne */
        if (sock_err != SOCKET_ERROR) {
            /* Démarrage du listage (mode server) */
            sock_err = listen(sock, 1);

            /* Si la socket fonctionne */
            if (sock_err != SOCKET_ERROR) {
                /* Attente pendant laquelle le client se connecte */
#ifdef _WITH_TRACE_
                printf("Waiting a client on port %d\n", port);
#endif
                csock = accept(sock, (SOCKADDR*) & csin, &crecsize);
#ifdef _WITH_TRACE_
                printf("Client connected\n");
#endif
                return 0;
            } else {
                perror("listen");
            }
        } else {
            perror("bind");
            return (-1);
        }
    } else {
        perror("socket");
        return (-1);
    }
    return 0;
}

int close_server(void) {
    /* Fermeture de la socket client et de la socket serveur */
    close(csock);
    close(sock);
    printf("Socket fermé\n");
    return 0;
}

int send_message_to_monitor(const char* typeMessage, const void * data) {
    if ((string) typeMessage == HEADER_STM_IMAGE) {
        Jpg * imgC = (Jpg*) data;
        serverSend("IMG", 3);
        serverSend(imgC->data(), imgC->size());
        serverSend("TRAME", 5);
        return 0;
    } else if ((string) typeMessage == HEADER_STM_POS) {
        char buffer[20];
        Position * maPosition = (Position*) data;
        sprintf(buffer, "POScenter: %3d;%3d | %.1fTRAME", maPosition->center.x, maPosition->center.y, maPosition->angle);
        serverSend(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_MES) {
        char buffer[50];
        sprintf(buffer, "MSG%sTRAME", (const char*) data);
        serverSend(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_ACK) {
        char buffer[50];
        sprintf(buffer, "ACK%sTRAME", (const char*) data);
        serverSend(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_BAT) {
        char buffer[50];
        sprintf(buffer, "BAT%sTRAME", (const char*) data);
        serverSend(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_NO_ACK) {
        char buffer[50];
        sprintf(buffer, "NAK%sTRAME", (const char*) data);
        serverSend(buffer, strlen(buffer));
        return 0;
    } else if ((string) typeMessage == HEADER_STM_LOST_DMB) {
        char buffer[50];
        sprintf(buffer, "LCD%sTRAME", (const char*) data);
        serverSend(buffer, strlen(buffer));
        return 0;
    } else {
        return -1;
    }
}

int receive_message_from_monitor(char *typeMessage, char *data) {
    char buffer[20];
    int tBuffer = receive(buffer);
    sscanf(buffer, "%3s:%s", typeMessage, data);
    return tBuffer;
}

int receive(char *data) {
    int result;
    result = recv(csock, data, 20, 0);
    //cout <<"Data : " << data << endl;
    return result;
}

int serverSend(const void *data, int dataLength) {
    if (send(csock, data, dataLength, 0) != dataLength) {
        perror("Send failed : ");
        return false;
    }

    return true;
}
