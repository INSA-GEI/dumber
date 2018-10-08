/**
 ******************************************************************************
 * @file    cmdManager.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    16-mai-2016
 * @brief   Gestion de commande reçu via l'uart
 *			Traite les chaine de caractére reçu par l'uart.
 *			Permet de verifier les erreurs de checksum,
 *			de traiter les valeurs retour.
 *
 *@attention Utilise les variables global - receiptString - sendString
 *
 ******************************************************************************
 ******************************************************************************
 */

#include <stm32f10x.h>

#include "cmdManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmde_usart.h"
#include "battery.h"
#include "motor.h"
#include "system_dumby.h"

/** @addtogroup Projects
 * @{
 */

/** @addtogroup cmdManager
 * @{
 */

volatile unsigned char checksum;
volatile int length;
uint16_t j;

/** @addtogroup Checksum
 * @{
 */
/**
 * @brief 		Inclut le checksum à sendString
 *
 *				Parcours sendString pour y calculer le checksum ( xor sur chaque caractére)
 *				et inclut le resultat en fin de chaine.
 *
 * @param  		None
 * @retval 		0 ou 1
 *
 */
void inclusionCheckSum(void) {
    checksum = 0;
    for (j = 0; sendString[j] != '\r'; j++)
        checksum ^= sendString[j];
    if (checksum == '\r')
        checksum++;
    sendString[j] = checksum;
    sendString[j + 1] = '\r';
}

/**
 * @brief 		Verifie le checksum de la variable global recepitString
 *
 * 				Vérifie le dernier carctére de receiptString sensé être le checksum.
 *				Si celui-ci est bon, ll retournera 0 et supprimera le checksum du tableau receiptString
 * 				sinon il retournera 1 sans faire de modification.
 * @param  		None
 * @retval 		0 ou 1
 *
 */
char verifyCheckSum(void) {
    uint16_t j, lenght;
    checksum = 0;
    lenght = strlen(receiptString);
    for (j = 0; j < lenght - 2; j++) {
        checksum ^= receiptString[j];
    }
    if (checksum == '\r')
        checksum++;

    if (receiptString[j] == checksum) {
        receiptString[lenght - 2] = 13;
        receiptString[lenght - 1] = 0;
        receiptString[lenght] = 0;
        return 0;
    } else
        return 1;
}

/**
 * @}
 */


/** @addtogroup TestCmd
 * @{
 */
/**
 * @brief 		Traite la commande de reception
 *
 * 				Gére le premier caractére de la chaine de reception pour en déduire la
 * 				commande à appliqué.
 *				Copie E dans sendString si une erreur s'est produite.
 *				Copie C dans sendString si la commande n'a pas abouti.
 *
 * @param  		None
 * @retval 		None
 */

void manageCmd(void) {
    switch (receiptString[0]) {
        case PingCMD:
            actionPing();
            break;

        case ResetCMD:
            actionReset();
            break;

        case StartWWatchDogCMD:
            actionStartWithWD();
            break;

        case ResetWatchdogCMD:
            actionResetWD();
            break;

        case GetBatteryVoltageCMD:
            actionBatteryVoltage();
            break;

        case GetVersionCMD:
            actionVersion();
            break;

        case StartWithoutWatchCMD:
            actionStartWWD();
            break;

        case MoveCMD:
            actionMove();
            break;

        case TurnCMD:
            actionTurn();
            break;

        case BusyStateCMD:
            actionBusyState();
            break;

        case 'a':
            actionDebug();
            break;

        default:
            strcpy(sendString, UNKNOW_ANS);
    }
}

/**
 * @}
 */


/** @addtogroup Traitement_Cmd
 * @{
 */
/**
 * @brief 		Commande Ping
 * 				Retourne OK_ANS si success dans sendString. Sinon return ERR_ANS.
 *
 * @param  		None
 * @retval 		None
 */

void actionPing(void) {
    if (receiptString[1] == 13)
        strcpy(sendString, OK_ANS);
    else
        strcpy(sendString, ERR_ANS);
}

/**
 * @brief 		Execute la commande reset.
 * 				Remet l'état de dumby à "idle".
 * 				Retourne OK_ANS si success dans sendString. Sinon return ERR_ANS.
 *
 * @param  		None
 * @retval 		None
 */
void actionReset(void) {
    Dumber.StateSystem = IDLE;
    Dumber.WatchDogStartEnable = TRUE;
    Dumber.cpt_watchdog = 0;
    Dumber.cpt_systick = 0;
    cmdLeftMotor(BRAKE, 0);
    cmdRightMotor(BRAKE, 0);
    strcpy(sendString, OK_ANS);
}

/**
 * @brief 		Execute la commande Version.
 *				Retourne la version du soft dans sendString.
 * @param  		None
 * @retval 		None
 */
void actionVersion(void) {
    if (receiptString[1] == 13)
        strcpy(sendString, VERSION);
    else
        strcpy(sendString, ERR_ANS);
}

/**
 * @brief 		Effectue une action BusyState.
 * 				Retourne si le robot est en mouvement ou non.
 * 				La variable est passé dans sendString - O ou 1.
 *
 * @param  		None
 * @retval 		None
 */
void actionBusyState(void) {
    if (Dumber.StateSystem == RUN || Dumber.StateSystem == LOW) {
        if (Dumber.busyState == TRUE)
            strcpy(sendString, "1");
        else
            strcpy(sendString, "0");
    } else {
        strcpy(sendString, ERR_ANS);
    }
}

/**
 * @brief 		Effectue une remise à zero du watchdog.
 *
 * @param  		None
 * @retval 		None
 */
void actionResetWD(void) {
    if (Dumber.StateSystem == RUN && watchDogState==TRUE){
        Dumber.cpt_watchdog = 0;
        strcpy(sendString, OK_ANS);
    }
    else
        strcpy(sendString, ERR_ANS);
}

/**
 * @brief 		Passe le robot en mode RUN.
 *				Necessite que le robot soit en mode IDLE au préalable.
 *				Le passage en mode run activera également les timers du watchdog.
 *				Qu'il faut remettre à zero entre 950ms et 1050ms.
 *
 * @param  		None
 * @retval 		None
 */
void actionStartWithWD(void) {
    if (Dumber.StateSystem == IDLE && receiptString[1] == 13) {
        strcpy(sendString, OK_ANS);
        Dumber.WatchDogStartEnable = TRUE;
        Dumber.StateSystem = RUN;
    } else
        strcpy(sendString, ERR_ANS);
}

/**
 * @brief 		Passe le robot en mode RUN mais sans watchdog.
 *				Necessite que dumby soit en mode IDLE au prealable.
 *
 * @param  		None
 * @retval 		None
 */
void actionStartWWD(void) {
    if (Dumber.StateSystem == IDLE && receiptString[1] == 13) {
        strcpy(sendString, OK_ANS);
        Dumber.WatchDogStartEnable = FALSE;
        Dumber.StateSystem = RUN;
    } else
        strcpy(sendString, ERR_ANS);
}


/**
 * @brief 		Demande un mouvement en ligne droite.
 * 				Les paramétres sont inclus dans receiptString.
 *				Le type de commande à envoyer est :"M=val\r". Ou val
 *				peut être positif ou negatif.
 *
 * @param  		None
 * @retval 		None
 */
void actionMove(void) {
    if (Dumber.StateSystem == RUN || Dumber.StateSystem == LOW) {
        int laps;
        uint16_t testReception = sscanf(receiptString, "M=%i\r", &laps);
        unsigned char mod = 0;
        tourPositionG = 0;
        tourPositionD = 0;

        if (testReception == 1) {
            Dumber.cpt_inactivity = 0;
            Dumber.busyState = TRUE;
            if (laps < 0) {
                laps = laps * -1;
                mod = REVERSE;
            } else
                mod = FORWARD;

            laps = laps * 2;
            regulationMoteur(mod, mod, (unsigned) laps, (unsigned) laps,
                    COMMONSPEED, COMMONSPEED);
            strcpy(sendString, OK_ANS);
        } else
            strcpy(sendString, ERR_ANS);
    }
}

/**
 * @brief 		Execute une action tourne avec les paramétres dans receitpString.
 * 				Type de commande à envoyer : "T=val\r". Ou val peut être positif
 * 				ou negatif.
 *
 * @param  		None
 * @retval 		None
 */
void actionTurn(void) {
    if (Dumber.StateSystem == RUN || Dumber.StateSystem == LOW) {
        int degree;
        uint16_t testReception = sscanf(receiptString, "T=%i\r", &degree);
        tourPositionG = 0;
        tourPositionD = 0;
        if (testReception == 1) {
            degree = degree * 1.40;
            Dumber.cpt_inactivity = 0;
            Dumber.busyState = TRUE;
            if (degree < 0) {
                degree = degree * -1;
                if (degree < 30)
                    regulationMoteur(FORWARD, REVERSE, (unsigned) degree,
                            (unsigned) degree, LOWSPEED, LOWSPEED);
                else
                    regulationMoteur(FORWARD, REVERSE, (unsigned) degree,
                            (unsigned) degree, COMMONSPEED, COMMONSPEED);
            } else {
                if (degree < 30)
                    regulationMoteur(REVERSE, FORWARD, (unsigned) degree,
                            (unsigned) degree, LOWSPEED, LOWSPEED);
                else
                    regulationMoteur(REVERSE, FORWARD, (unsigned) degree,
                            (unsigned) degree, COMMONSPEED, COMMONSPEED);
            }
        } else
            strcpy(sendString, ERR_ANS);
    }
}

/**
 * @brief		retourne une valeur de batterie.
 * 				La valeur de la batterie peut être - 0 - 1 - 2.
 * 				Lorsque la batterie est pleine, la valeur retournée est 2.
 * 				Lorque la batterie est intermediaire, la valeur retournée est 1.
 *				Lorsque la batterie est vide, la valeur retournée est 0.
 *
 * @param  		None
 * @retval 		None
 */
void actionBatteryVoltage(void) {
    char battery[2];
    battery[0] = Dumber.stateBattery + '0';
    battery[1] = '\r';
    strcpy(sendString, battery);
}

/**
 * @brief		Envoie le nombre de tour théorique à effectuer.
 * 				Et le nombre de tour effectué au moment de reception de la commande.
 * 				Th-  Consigne de commande en nombre de tour.
 *				Re-	 Nombre de tour reel effectué.
 *
 * @param  		None
 * @retval 		None
 */
void actionDebug(void) {
    sprintf(sendString, "Th-D=%u G=%u\r", tourPositionD, tourPositionG);
    sendDataUSART();
    for (j = 0; j < 200; j++);
    sprintf(sendString, "Re-D=%u G=%u\r", G_lapsRight, G_lapsLeft);
    sendDataUSART();
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

