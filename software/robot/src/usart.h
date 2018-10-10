/**
 ******************************************************************************
 * @file    cmd_uart.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    19-June-2017
 * @brief   Gestion de l'uart
 * @note Ce fichier contient les fonctions lié à la gestion de la communications avec l'USART :
 * - Fonction definissant les E/S lié à l'uart.
 * - Initialisation de la dma pour l'envoi
 * - Initialisation de façon non bloquante de la reception en polling
 * - Gestions des commandes
 ******************************************************************************
 ******************************************************************************
 */
#ifndef _USART_H_
#define _USART_H_

#include "stm32f10x.h"

#define TBuffer   30

/* Variables externes nécessaire*/
extern char sendString[TBuffer];
extern char receiptString[TBuffer];
extern uint16_t cpt_Rx;

/* Prototype des fonctions */
void INIT_IT_UsartReceive(void);
void usartInitDMA(uint16_t bufferSize);
void usartSendData(void);
void usartConfigure(void);
void MAP_UsartPin(void);

#endif /* _USART_H_ */

