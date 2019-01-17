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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmdManager.h"
#include "battery.h"
#include "motor.h"
#include "system_dumby.h"
#include "usart.h"

/** @addtogroup Projects
 * @{
 */

/** @addtogroup cmdManager
 * @{
 */

/* Definition des commandes */

#define PingCMD                 'p'
#define ResetCMD                'r'
#define SetMotorCMD             'm'
#define StartWWatchDogCMD       'W'
#define ResetWatchdogCMD        'w'
#define GetBatteryVoltageCMD    'v'
#define GetVersionCMD           'V'
#define StartWithoutWatchCMD    'u'
#define MoveCMD                 'M'
#define TurnCMD                 'T'
#define BusyStateCMD            'b'
#define TestCMD                 't'
#define DebugCMD                'a'
#define PowerOffCMD             'z'

#define OK_ANS              "O\r"
#define ERR_ANS             "E\r"
#define UNKNOW_ANS          "C\r"
#define BAT_OK              "2\r"
#define BAT_LOW             "1\r"
#define BAT_EMPTY           "0\r"

/* Prototype des fonctions */

char cmdVerifyChecksum(void);
void cmdAddChecksum(void);
void cmdResetAction(void);
void cmdBusyStateAction(void);
void cmdPingAction(void);
void cmdVersionAction(void);
void cmdStartWithoutWatchdogAction(void);
void cmdMoveAction(void);
void cmdTurnAction(void);
void cmdBatteryVoltageAction(void);
void cmdStartWithWatchdogAction(void);
void cmdResetWatchdogAction(void);
void cmdDebugAction(void);
void cmdPowerOffAction(void);

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
void cmdAddChecksum(void) {
    uint16_t j;
    unsigned char checksum=0;

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
char cmdVerifyChecksum(void) {
    uint16_t j;
    uint16_t length;
    unsigned char checksum=0;

    length = strlen(receiptString);
    for (j = 0; j < length - 2; j++) {
        checksum ^= receiptString[j];
    }
    if (checksum == '\r')
        checksum++;

    if (receiptString[j] == checksum) {
        receiptString[length - 2] = 13;
        receiptString[length - 1] = 0;
        receiptString[length] = 0;

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

void cmdManage(void) {
    if (cmdVerifyChecksum() != 0) {
        strcpy(sendString, UNKNOW_ANS);
    } else { // Checksum valide
        if (Dumber.StateSystem==STATE_DISABLE) { // SI la batterie est trop faible, impossible d'accepter une commande sauf poweroff: on reste dans ce mode
            if (receiptString[0]==PowerOffCMD)
                cmdPowerOffAction();
            else
                strcpy(sendString, ERR_ANS);
        } else {
            switch (receiptString[0]) {
                case PingCMD:
                    cmdPingAction();
                    break;

                case ResetCMD:
                    cmdResetAction();
                    break;

                case StartWWatchDogCMD:
                    cmdStartWithWatchdogAction();
                    break;

                case ResetWatchdogCMD:
                    cmdResetWatchdogAction();
                    break;

                case GetBatteryVoltageCMD:
                    cmdBatteryVoltageAction();
                    break;

                case GetVersionCMD:
                    cmdVersionAction();
                    break;

                case StartWithoutWatchCMD:
                    cmdStartWithoutWatchdogAction();
                    break;

                case MoveCMD:
                    cmdMoveAction();
                    break;

                case TurnCMD:
                    cmdTurnAction();
                    break;

                case BusyStateCMD:
                    cmdBusyStateAction();
                    break;

                case DebugCMD:
                    cmdDebugAction();
                    break;

                case PowerOffCMD:
                    cmdPowerOffAction();
                    break;
                default:
                    strcpy(sendString, UNKNOW_ANS);
            }
        }
    }

    Dumber.cpt_inactivity=0; // remise a zéro du compteur d'inativité
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

void cmdPingAction(void) {
    if (receiptString[1] == '\r')
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
void cmdResetAction(void) {
    systemChangeState(STATE_IDLE);
    strcpy(sendString, OK_ANS);
}

/**
 * @brief 		Execute la commande Version.
 *				Retourne la version du soft dans sendString.
 * @param  		None
 * @retval 		None
 */
void cmdVersionAction(void) {
    if (receiptString[1] == '\r')
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
void cmdBusyStateAction(void) {
    if ((Dumber.StateSystem == STATE_RUN) || (Dumber.StateSystem == STATE_LOW)) {
        if (Dumber.busyState == TRUE)
            strcpy(sendString, "1\r");
        else
            strcpy(sendString, "0\r");
    } else {
        strcpy(sendString, ERR_ANS);
    }
}

/**
 * @brief 		Effectue une remise à zéro du watchdog.
 *
 * @param  		None
 * @retval 		None
 */
void cmdResetWatchdogAction(void) {
    if (systemResetWatchdog()!=0) { // Réussite
        strcpy(sendString, OK_ANS);
    } else strcpy(sendString, ERR_ANS);
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
void cmdStartWithWatchdogAction(void) {
    if (Dumber.StateSystem == STATE_IDLE && receiptString[1] == '\r') {
        Dumber.WatchDogStartEnable = TRUE;
        systemChangeState(STATE_RUN);
        strcpy(sendString, OK_ANS);
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
void cmdStartWithoutWatchdogAction(void) {
    if (Dumber.StateSystem == STATE_IDLE && receiptString[1] == '\r') {
        Dumber.WatchDogStartEnable = FALSE;
        systemChangeState(STATE_RUN);
        strcpy(sendString, OK_ANS);
    } else
        strcpy(sendString, ERR_ANS);
}


/**
 * @brief 		Demande un mouvement en ligne droite.
 * 				Les paramétres sont inclus dans receiptString.
 *				Le type de commande à envoyer est :"M=val\r". Ou val
 *				peut être positif ou negatif.
 *
 * @param  		NSTART_WITH_WDone
 * @retval 		None
 */
void cmdMoveAction(void) {
    if (Dumber.StateSystem == STATE_RUN || Dumber.StateSystem == STATE_LOW) {
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

            motorRegulation(mod, mod, (unsigned) laps, (unsigned) laps,
                    COMMONSPEED, COMMONSPEED);

            strcpy(sendString, OK_ANS);
        } else
            strcpy(sendString, ERR_ANS);
    }
}

/**
 * @brief 		Execute une action tourne avec les paramètres dans receitpString.
 * 				Type de commande à envoyer : "T=val\r". Ou val peut être positif
 * 				ou negatif.
 *
 * @param  		None
 * @retval 		None
 */
void cmdTurnAction(void) {
    if (Dumber.StateSystem == STATE_RUN || Dumber.StateSystem == STATE_LOW) {
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
                motorRegulation(FORWARD, REVERSE, (unsigned) degree,
                        (unsigned) degree, LOWSPEED, LOWSPEED);
            } else {
                motorRegulation(REVERSE, FORWARD, (unsigned) degree,
                        (unsigned) degree, LOWSPEED, LOWSPEED);
            }
            strcpy(sendString, OK_ANS);
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
void cmdBatteryVoltageAction(void) {
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
void cmdDebugAction(void) {
    uint8_t j;

    sprintf(sendString, "Th-D=%u G=%u\r", tourPositionD, tourPositionG);
    usartSendData();

    for (j = 0; j < 200; j++);
    sprintf(sendString, "Re-D=%u G=%u\r", G_lapsRight, G_lapsLeft);
    usartSendData();
}

/**
 * @brief       Eteint le robot
 *
 * @param       None
 * @retval      None
 */
void cmdPowerOffAction(void) {
    volatile int i;

    systemChangeState(STATE_DISABLE);
    strcpy(sendString, OK_ANS);

    cmdAddChecksum();
    usartSendData();
    /* Attente d'un certain temps (100 ms), pour que la reponse parte */
    for (i=0; i<100000; i++);

    systemShutDown(); // Ne ressort jamais de cette fonction
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

