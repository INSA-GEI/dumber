/**
 ******************************************************************************
 * @file    cmdManager.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    19-June-2017
 * @brief   Gestion de commande reçu via l'uart
 *
 *			Traite les chaînes de caractère reçu par l'uart.
 *			Permet de vérifier les erreurs de checksum,
 *			de traiter les valeurs retours.
 *
 * @attention Utilise les variables globales - receiptString - sendString
 *
 ******************************************************************************
 ******************************************************************************
 */

#ifndef _CMD_MANAGER_H_
#define _CMD_MANAGER_H_

#include <stm32f10x.h>

void cmdManage(void);
void cmdAddChecksum(void);

#endif /* _CMD_MANAGER_H_ */

