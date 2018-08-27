#include "robot.h"

int fd;

int getChar(char * c);
int readSerial(char * msg);
char checkSumGO(char * msg);
int receiveMsg(void);
int sendCmd(char cmd, const char * arg);

int open_communication_robot(const char * path)
{
#ifndef __STUB__
    struct termios options;
    fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd !=-1)
    {
        fcntl(fd, F_SETFL, 0);
        tcgetattr(fd, &options);
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        cfsetospeed (&options, B9600);
        cfsetispeed (&options, B9600);
        options.c_cc[VMIN]=0;
        options.c_cc[VTIME]=0;
        tcsetattr(fd, TCSANOW, &options);
        return 0;
    }
    else
    {
        perror("can't openSerial");
        return -1;
    }
#else
    return 0;
#endif
}


int close_communication_robot(void)
{
#ifndef __STUB__
    return close(fd);
#else
    return 0;
#endif
}


int send_command_to_robot(char cmd, const char * arg)
{
#ifndef __STUB__
    sendCmd(cmd,arg);
    // TODO : check return from sendCmd
    return receiveMsg();
#else
    int reponse;
    switch(cmd)
    {
        case DMB_PING:
            reponse = 0;
            break;
        case DMB_IDLE:
            reponse = 0;
            break;
        case DMB_START_WITH_WD:
            reponse = 0;
            break;
        case DMB_RELOAD_WD:
            reponse = 0;
            break;
        case DMB_GET_VBAT:
            reponse = 2;
            break;
        case DMB_IS_BUSY:
            reponse = 1;
            break;
        case DMB_START_WITHOUT_WD:
            reponse = 0;
            break;
        case DMB_MOVE:
            reponse = 0;
            break;
        case DMB_TURN:
            reponse = 0;
            break;
        case DMB_GO_FORWARD:
            reponse = 0;
            break;
        case DMB_GO_BACK:
            reponse = 0;
            break;
        case DMB_GO_LEFT:
            reponse = 0;
            break;
        case DMB_GO_RIGHT:
            reponse = 0;
            break;
        case DMB_STOP_MOVE:
            reponse = 0;
            break;
        default:
            reponse = 0;
            break;
    }
    return reponse;
#endif
}

/****************************/
/* PRIVATE                  */
/****************************/

int sendCmd(char cmd, const char * arg)
{
    char cmdWithArg[20]={};
    cmdWithArg[0]=cmd;
    switch(cmd)
    {
        case DMB_GO_FORWARD: strcpy(cmdWithArg,"M=+64000");
            break;
        case DMB_GO_BACK:    strcpy(cmdWithArg,"M=-64000");
            break;
        case DMB_GO_LEFT:  strcpy(cmdWithArg,"T=+64000");
            break;
        case DMB_GO_RIGHT: strcpy(cmdWithArg,"T=-64000");
            break;
        case DMB_STOP_MOVE:  strcpy(cmdWithArg,"M=0");
            break;
        case DMB_MOVE:   strcat(cmdWithArg,"=");
            strcat(cmdWithArg,arg);
            break;
        case DMB_TURN:   strcat(cmdWithArg,"=");
            strcat(cmdWithArg,arg);
            break;
    }
    int sizeCmd = strlen(cmdWithArg);
    cmdWithArg[sizeCmd] = checkSumGO(cmdWithArg);
    cmdWithArg[sizeCmd+1] = '\r';
    cmdWithArg[sizeCmd+2] = '\0';
    return write(fd,cmdWithArg,strlen(cmdWithArg));
}

int receiveMsg(void)
{
    char msg[20];
    int b;
    if((b = readSerial(msg))!=ROBOT_TIMED_OUT)
    {
        int taille = strlen(msg);
        char checksum = msg[taille-2];
        msg[taille-1] = 0;
        msg[taille-2] = 0;
        if(checksum!=checkSumGO(msg))
        {
            return ROBOT_CHECKSUM;
        }
        else
        {
            switch(msg[0])
            {
                case 'O' : return 0;
                case 'E' : return ROBOT_ERROR;
                case 'C' : return ROBOT_UKNOWN_CMD;
                default :  return atoi(&msg[0]);
            }
        }
    }
    else
    {
        return ROBOT_TIMED_OUT;
    }
}

int getChar(char * c)
{
    int n =0;
    int delay =0;
    while((n=read(fd,c,1)) <=0)
    {
        usleep(5000);
        delay++;
        if(delay > 10)
        {
            return ROBOT_TIMED_OUT;
        }

    }
    return n;
}

int readSerial(char * msg)
{
    char car;
    int i =0,j=0;
    for(j = 0 ; j < 20 ; j++)
        msg[j]=0;
    while(car !='\r' && car!='\n')
    {
        if(i>=20)
            return -5;

        if(getChar(&car)==ROBOT_TIMED_OUT)
        {
            return ROBOT_TIMED_OUT;
        }
        msg[i] = car;
        i++;
    }
    return i;
}

char checkSumGO(char * msg)
{
    char resultat = 0;
    int i = 0;
    int taille = strlen(msg);
    for(i=0;i<taille;i++)
    {
        resultat^=msg[i];
    }
    return resultat;

}



