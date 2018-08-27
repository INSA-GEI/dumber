#include <task.h>
#include "../src/imagerie.h"
#include "../src/serial.h"
#include "../src/tcpServer.h" // include himself imagerie.h


using namespace std;
using namespace cv;
using namespace raspicam;

RT_TASK video;

void threadVideo(void *arg)
{
	printf("Thread lancé ... \n");
	Camera rpiCam;
	Image imgVideo;
	Arene monArene;
	position positionRobots[20];
	Jpg compress;

	openCamera(&rpiCam);
	do
	{
	   getImg(&rpiCam, &imgVideo);
	   if(detectArena(&imgVideo, &monArene)==0)
	   {
	      detectPosition(&imgVideo,positionRobots,&monArene);
	      drawArena(&imgVideo,&imgVideo,&monArene);
	   }
	   else
	      detectPosition(&imgVideo,positionRobots);

	   drawPosition(&imgVideo,&imgVideo,&positionRobots[0]);
	   imgCompress(&imgVideo,&compress);

	   sendToUI("IMG",&compress);
	   sendToUI("POS",&positionRobots[0]);
	}while(waitKey(30)!='q');
	closeCam(&rpiCam);
}



int main() {
    serverOpen();
    robotOpenCom();

    char header[4];
    char data[20];
    memset(data, '\0',20);
    memset(header,'\0',4);

    
    if(rt_task_spawn(&video,"envoieVideo",0,20,0, threadVideo, NULL) == -1)
	perror("erreur lors de la creation du thread\n");

    do
    {   receptionFromUI(header,data);
        if(strcmp(header, DMB) == 0)
        {
            printf("EVENEMENT DUMBER DETECTE AVEC LE MESSAGE :%s \n",data);
            int a = robotCmd(data[0]);
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
    robotCloseCom();
    serverClose();

    return 0;
}
