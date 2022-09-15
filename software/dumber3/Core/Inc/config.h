/*
 * config.h
 *
 *  Created on: 14 sept. 2022
 *      Author: dimercur
 */
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"

#define STACK_SIZE 					100

#define PriorityMoteurs        		(configMAX_PRIORITIES -1)
#define PriorityXbee 				(configMAX_PRIORITIES -2)
#define PriorityBatterie			(configMAX_PRIORITIES -3)
#define PrioritySequenceur			(configMAX_PRIORITIES -4)
#define PriorityLeds    			(configMAX_PRIORITIES -5)

TickType_t msToTicks(TickType_t ms);
