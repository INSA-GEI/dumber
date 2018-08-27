#include "system_dumby.h"
#include <stm32f10x.h>
#include <string.h>
#include "cmde_usart.h"
#include "Battery.h"
#include "motor.h"
#include <stdlib.h>
#include "gestionCmde.h"
#include <stdio.h>


 /*
	*	Ce fichier gére les commandes reçu dans la variable receiptString
	* Les données envoyé le seront via la variable sendString
	*/


volatile unsigned char checksum;
volatile int length;
uint16_t j;

 /* inclusionCheckSum:
	* incusionCheckSum(void) inclu le checksum (ou exclusif sur chaque octet) 
	* à la fin de la chaine de caractére sendString
	*/

void inclusionCheckSum(void)
{
	checksum=0;
	for(j=0;sendString[j]!='\r';j++)
		checksum^=sendString[j];
	if(checksum=='\r')
		checksum++;
	sendString[j]=checksum;
	sendString[j+1]='\r';
}

 /* verifyCheckSum :
	* Vérifie le dernier carctére de receiptString sensé être le checksum.
	* Si celui-ci est bon, ll retournera 0 et supprimera le checksum du tableau receiptString
	* sinon il retournera 1 sans faire de modification.
	*/
// verifie le checksum. Si le checksum est verifié on l'enléve de la chaine de caractére pour facilité le traitement.
char verifyCheckSum(void)
{	
	uint16_t j,lenght;
	checksum=0;
	lenght = strlen(receiptString);
	for(j=0;j<lenght-2;j++)
	{
		checksum^=receiptString[j];
	}
	if(checksum == '\r')
		checksum++;
	
	if(receiptString[j]==checksum)
	{
		receiptString[lenght-2]=13;
		receiptString[lenght-1]=0;
		receiptString[lenght]=0;
		return 0;
	}
	else
		return 1;
}


 /* manageCmd :
	* Gére le premier caractére de la chaine de reception pour en déduire la commande à appliqué. 
	* Renvoi O si tout s'est bien passé
	*				E si une erreur s'est produite
	*				C si la commande n'a pas abouti
	*/


void manageCmd(void)
{
	switch(receiptString[0])
	{
		case PingCMD : 									actionPing();
							break;
		
		case ResetCMD :									actionReset();
							break;
		
		case StartWWatchDogCMD:					actionStartWithWD();
							break;
		
		case ResetWatchdogCMD:					actionResetWD();
							break;
		
		case GetBatteryVoltageCMD :			actionBatteryVoltage();
							break;
		
		case GetVersionCMD :						actionVersion();
							break;
		
		case StartWithoutWatchCMD:	 		actionStartWWD();
							break;
							
		case MoveCMD :									actionMove();
							break;
							
		case TurnCMD :									actionTurn();
							break;				

		case BusyStateCMD:							actionBusyState();
							break;
		case 'a':												actionDebug();
							break;
		
		default:
			strcpy(sendString, UNKNOW_ANS);	
	}
}

 /* actionPing:
	* Action ping permet de verifier la connection avec le robot. Renvoi OK si le robot est connecté.
	* Renvoi ERR_ANS sinon.
	*/
void actionPing(void)
{
	if(receiptString[1] == 13)
		strcpy(sendString, OK_ANS);
	else
		strcpy(sendString,ERR_ANS);
}

 /*
	* Action reset permet de replacer dumber dans 
	* l'état IDLE peut importe l'etat courant de celui-ci.
	* Il remetra à zero les valeurs de temps de dumber et coupera les moteurs
	*/
void actionReset(void)
{
	Dumber.StateSystem = IDLE;
	Dumber.WatchDogStartEnable = TRUE;
	Dumber.cpt_watchdog=0;
	Dumber.cpt_systick=0;
	cmdLeftMotor(BRAKE,0);
	cmdRightMotor(BRAKE,0);
	strcpy(sendString, OK_ANS);
}

 /*actionVersion:
	* Retourne la version de dumber
	*/
void actionVersion(void)
{
	if(receiptString[1] == 13)
	strcpy(sendString,VERSION);
	else
		strcpy(sendString,ERR_ANS);
}


 /* actionBusyState:
	* Retourne OK_ANS lorsque le robot est "busy" à savoir qu'il est en cours de deplacement;
	*/
void actionBusyState(void)
{
	if(Dumber.StateSystem==RUN || Dumber.StateSystem==LOW)
	{
		if(Dumber.busyState==TRUE)
			strcpy(sendString, "1");		
		else
			strcpy(sendString,"0");
	}
	else
	{
		strcpy(sendString,ERR_ANS);
	}
}

 /* actionResetWD:
	*  Remise à zero du cpt_watchdog - Retourne OK_ANS si effectué. 
	*/
void actionResetWD(void)
{
	if((Dumber.StateSystem==RUN || Dumber.StateSystem==LOW) && receiptString[1] == 13 && watchDogState==TRUE){
			Dumber.cpt_watchdog=0;
			strcpy(sendString,OK_ANS);
	}
	else
		strcpy(sendString,ERR_ANS);
}

 /* actionStartWithWD:
	* Passe à l'état IDLE ou RUN en activant le compteur watchdog
	* (remise à zero du compteur necessaire entre 950ms et 1050ms)
	*/
void actionStartWithWD(void)
{
	if(Dumber.StateSystem==IDLE && receiptString[1] == 13)
	{
		strcpy(sendString,OK_ANS);
		Dumber.WatchDogStartEnable = TRUE;
		Dumber.StateSystem = RUN;
	}
	else
		strcpy(sendString,ERR_ANS);
}


	/* actionStartWWD:
	* Passe à l'état IDLE ou RUN
	*/
void actionStartWWD(void)
{
	if(Dumber.StateSystem==IDLE && receiptString[1] == 13)
	{
		strcpy(sendString,OK_ANS);
		Dumber.WatchDogStartEnable = FALSE;
		Dumber.StateSystem = RUN;
	}
	else
		strcpy(sendString,ERR_ANS);
}

 /* actionMove:
	* Bouge dumber en ligne droite. Le paramétre se calcule en nombre d'interuption de capteur. 
	*/
void actionMove(void)
{
	if(Dumber.StateSystem==RUN||Dumber.StateSystem==LOW)
	{
		int laps;
		uint16_t testReception = sscanf(receiptString, "M=%i\r", &laps);
		unsigned char mod=0;
		tourPositionG=0;
		tourPositionD=0;
		
		if(testReception==1)
		{
			Dumber.cpt_inactivity=0;
			Dumber.busyState=TRUE;
			if(laps < 0)
			{
				laps =laps*-1;
				mod = REVERSE;
			}
			else if(laps > 0)
				mod = FORWARD;
			else
				mod = BRAKE;
			
			laps = laps *2;
			regulationMoteur(mod,mod,(unsigned)laps,(unsigned)laps,COMMONSPEED, COMMONSPEED);
			strcpy(sendString,OK_ANS);
		}
		else
			strcpy(sendString,ERR_ANS);	
	}
}

 /* actionTurn:
	* Fait tourner les 2 roues dans un sens opposé de façon à faire tourner le robot de X degrés.
	* Attention : Mauvais précision sur les petite valeur < 30 degrés.
	*/
void actionTurn(void)
{
	if(Dumber.StateSystem==RUN||Dumber.StateSystem==LOW)
	{
		int degree;
		uint16_t testReception = sscanf(receiptString, "T=%i\r",&degree);
		tourPositionG=0;
		tourPositionD=0;
		if(testReception==1)
		{
			degree = degree *1.40;
			Dumber.cpt_inactivity=0;
			Dumber.busyState=TRUE;
			strcpy(sendString,OK_ANS);
			if(degree <0)
			{
				degree = degree*-1;
				if(degree < 30)
					regulationMoteur(FORWARD,REVERSE,(unsigned)degree,(unsigned)degree,LOWSPEED, LOWSPEED);
				else
					regulationMoteur(FORWARD,REVERSE,(unsigned)degree,(unsigned)degree,COMMONSPEED, COMMONSPEED);
			}
			else
			{
				if(degree < 30)
					regulationMoteur(REVERSE,FORWARD,(unsigned)degree,(unsigned)degree,LOWSPEED, LOWSPEED);
				else
					regulationMoteur(REVERSE,FORWARD,(unsigned)degree,(unsigned)degree,COMMONSPEED, COMMONSPEED);
			}
			
		}
		else
			strcpy(sendString,ERR_ANS);
	}
}

 /* actionBatteryVoltage: 
	* Retourne l'état de la batterie. 0 - 1 - 2
  */

void actionBatteryVoltage(void)
{
	char battery[2];
	battery[0]=Dumber.stateBattery + '0';
	battery[1]='\r';
	strcpy(sendString,battery);
}

 /* actionDebug:
	* retourne les valeurs positions à atteindre et la position atteinte;
	*/

void actionDebug(void)
{
	sprintf(sendString,"Th-D=%u G=%u\r",tourPositionD,tourPositionG);
	sendDataUSART();
	for(j=0;j<200;j++);
	sprintf(sendString,"Re-D=%u G=%u\r",G_lapsRight,G_lapsLeft);
	sendDataUSART();
}

