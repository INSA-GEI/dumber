/*
 * config.h
 *
 *  Created on: 14 sept. 2022
 *      Author: dimercur
 */
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"

#define SYSTEM_VERSION_STR			"version 2.0\r"

#define STACK_SIZE 					0x100

#define PriorityMoteurs        		(configMAX_PRIORITIES -1)
#define PriorityXbeeRX 				(configMAX_PRIORITIES -2)
#define PriorityXbeeTX 				(configMAX_PRIORITIES -3)
#define PriorityBatterie			(configMAX_PRIORITIES -4)
#define PrioritySequenceurMain		(configMAX_PRIORITIES -5)
#define PrioritySequenceurTimeout	(configMAX_PRIORITIES -6)
#define PriorityLeds    			(configMAX_PRIORITIES -7)

#define BATTERIE_PERIODE_SCRUTATION	1000
#define XBEE_TX_SEMAPHORE_WAIT		500
#define XBEE_RX_BUFFER_MAX_LENGTH	50

TickType_t msToTicks(TickType_t ms);
