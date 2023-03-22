/*
 * xbee_ll.h
 *
 *  Created on: Apr 6, 2022
 *      Author: dimercur
 */

#ifndef BSP_XBEE_XBEE_LL_H_
#define BSP_XBEE_XBEE_LL_H_

#include "stm32l4xx_hal.h"

typedef void (*RXCallback)(char* data, int length);

#define XBEE_USART							USART1
#define XBEE_RST_PIN						GPIO_PIN_8
#define XBEE_RST_PORT						GPIOA

#define XBEE_SLEEP_RQ_PIN					GPIO_PIN_11
#define XBEE_SLEEP_RQ_PORT					GPIOA

#define XBEE_SLEEP_PIN					    GPIO_PIN_3
#define XBEE_SLEEP_PORT					    GPIOB

#define XBEE_LL_OK							0
#define XBEE_LL_ERROR_USART_CFG				1
#define XBEE_LL_ERROR_TX					2
#define XBEE_LL_ERROR_RX					3
#define XBEE_LL_ERROR_RX_TIMEOUT			4

int XBEE_LL_ConfigureUart(USART_TypeDef* usart, uint32_t baudrate);
void XBEE_LL_ConfigureGPIO(void);
void XBEE_LL_ConfigureTimer(void);
int XBEE_LL_SendData(char* data, int length) ;
int XBEE_LL_ReceiveData(char* data, int length, int timeout);

#endif /* BSP_XBEE_XBEE_LL_H_ */
