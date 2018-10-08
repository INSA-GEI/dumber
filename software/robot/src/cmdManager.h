/**
 ******************************************************************************
 * @file    cmdManager.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    19-June-2017
 * @brief   Gestion de commande reçu via l'uart
 *
 *			Traite les chaines de caractére reçu par l'uart.
 *			Permet de verifier les erreurs de checksum,
 *			de traiter les valeurs retours.
 *
 *@attention Utilise les variables globals - receiptString - sendString
 *
 ******************************************************************************
 ******************************************************************************
 */

#ifndef CMD_MANAGER_H_
#define CMD_MANAGER_H_

#include <stm32f10x.h>

void manageCmd(void);
char verifyCheckSum(void);
void inclusionCheckSum(void);
void actionReset(void);
void actionBusyState(void);
void actionPing(void);
void actionVersion(void);
void actionStartWWD(void);
void actionMove(void);
void actionTurn(void);
void actionBatteryVoltage(void);
void actionStartWithWD(void);
void actionResetWD(void);
void actionDebug(void);

#endif /* CMD_MANAGER_H_ */

