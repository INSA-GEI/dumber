#include "../src/imagerie.h"
#include "../src/serial.h"
#include "../src/tcpServer.h" // include himself imagerie.h
#include <pthread.h>
#include <unistd.h>

using namespace std;
using namespace cv;
#ifndef __STUB__
using namespace raspicam;
#endif

void * threadVideo(void *arg)
{
	printf("Thread lancé ... \n");
	Camera rpiCam;
	Image imgVideo;
	Arene monArene;
	Position positionRobots[20];
	Jpg compress;

	openCamera(&rpiCam);
	do
	{
#ifndef __STUB__
	   getImg(&rpiCam, &imgVideo);
#else
           getImg(&rpiCam, &imgVideo, "/home/pehladik/C-TP-RT/src/mondrian22.jpeg");
#endif
	   /*if(detectArena(&imgVideo, &monArene)==0)
	   {
	      detectPosition(&imgVideo,positionRobots,&monArene);
	      drawArena(&imgVideo,&imgVideo,&monArene);
	   }
	   else
	      detectPosition(&imgVideo,positionRobots);

	   drawPosition(&imgVideo,&imgVideo,&positionRobots[0]);*/
	   imgCompress(&imgVideo,&compress);

	   sendToUI("IMG",&compress);
	   sendToUI("POS",&positionRobots[0]);
	}while(waitKey(30)!='q');
	closeCam(&rpiCam);
	pthread_exit(NULL);
}

void * threadClient(void *arg){
    char * args [] = {"nodejs", "/home/pehladik/Interface-TP-RT/interface.js", NULL};
    if (execv("/usr/bin/nodejs", args)== -1){
        perror("execv");
        return EXIT_FAILURE;
    }
    
}

int main() {
    pthread_t threadClient;
    if(pthread_create(&threadClient,NULL, threadClient, NULL) == -1)
	perror("erreur lors de la creation du thread\n");
    

    serverOpen();
    robotOpenCom();

    char header[4];
    char data[20];
    memset(data, '\0',20);
    memset(header,'\0',4);

    pthread_t thread1;
    if(pthread_create(&thread1,NULL, threadVideo, NULL) == -1)
	perror("erreur lors de la creation du thread\n");

    do
    {   receptionFromUI(header,data);
        printf("Msg reçu : %s %s", header, data);
        
        if(strcmp(header, DMB) == 0)
        {
            printf("EVENEMENT DUMBER DETECTE AVEC LE MESSAGE :%s \n",data);
            int a = sendCmdToRobot(data[0]);
            printf("Resultat CMD : %d \n", a);
            if(data[0] == WITHOUT_WD && a == 0)
            {
                sendToUI(ACK);
            }
            else if(data[0] == BACKIDLE && a == 0)
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
    robotCloseCom();
    serverClose();

    return 0;
}
