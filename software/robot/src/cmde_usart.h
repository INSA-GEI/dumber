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
#ifndef cmde_usart_H
#define cmde_usart_H

#include "stm32f10x.h"

#define TBuffer   30

/* Prototype des fonctions */

void INIT_IT_UsartReceive(void);
void INIT_DMASend(uint16_t bufferSize);
void sendDataUSART(void);
void INIT_USART(void);
void MAP_UsartPin(void);

/* Variable Externes necessair e*/

extern char sendString[TBuffer];
extern char receiptString[TBuffer];
extern uint16_t cpt_Rx;

#endif /* CMD_UART_H_ */

