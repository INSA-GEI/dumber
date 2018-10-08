#include "../src/serial.h"
#include "../src/tcpServer.h"
#include "../src/imagerie.h"
#include <unistd.h>
#include <pthread.h>


int main() {
    //startTrace();
    runNodejs("/usr/bin/nodejs", "/home/pehladik/Interface-TP-RT/interface.js");
    
    printf("Lancement serveur ... \n");
    serverOpen();
    //stopTrace();
    printf("Serveur lancé ... \n");
    robotOpenCom();

    char header[4];
    char data[20];
    memset(data, '\0',20);
    memset(header,'\0',4);
    int res;
    do
    {  
        res = receptionFromUI(header,data);
        printf ("res : %d\n", res);
        if(strcmp(header, DMB) == 0)
        {
            printf("EVENEMENT DUMBER DETECTE AVEC LE MESSAGE :%s \n",data);
            int a = sendCmdToRobot(data[0]);
            printf("Resultat CMD : %d \n", a);
            if(data[0] == 'u' && a == 0)
            {
                sendToUI(ACK);
            }
            if(data[0] == 'r' && a == 0)
            {
                sendToUI(ACK);
            }
        }

        if(strcmp(header, MES) == 0)
        {
            printf("EVENEMENT MESSAGE DETECTE AVEC LE MESSAGE :%s \n",data);
        }

        if(strcmp(header,POS)==0)
        {
            printf("EVENEMENT POSITION DETECTE AVEC LE MESSAGE :%s \n",data);
        }
    }while((strcmp(header,MES)!=0) || (data[0] != 'C'));
    killNodejs();
    
    robotCloseCom();
    serverClose();
    pause();
    return 0;
}
