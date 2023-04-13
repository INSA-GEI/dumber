/*
 * config.h
 *
 *  Created on: 14 sept. 2022
 *      Author: dimercur
 */
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"

// current version is 2.0
#define SYSTEM_VERSION_STR			"2.0"
#define SYSTEM_VERSION				0x0200	// Upper byte: major version, lower byte: minor version

#define STACK_SIZE 					0x100

#define PriorityMoteursAsservissement	(configMAX_PRIORITIES -1)
#define PriorityXbeeRX 				(configMAX_PRIORITIES -2)
#define PriorityXbeeTX 				(configMAX_PRIORITIES -3)
#define PriorityMoteurs				(configMAX_PRIORITIES -4)
#define PriorityBatterie			(configMAX_PRIORITIES -5)
#define PriorityApplicationMain		(configMAX_PRIORITIES -6)
#define PrioritySequenceurTimeout	(configMAX_PRIORITIES -7)
#define PriorityLeds    			(configMAX_PRIORITIES -8)

/* Périodes des taches périodiques (en ms) */
#define MOTEURS_PERIODE_ASSERVISSEMENT	(2)
#define APPLICATION_PERIODE				(100)
#define BATTERIE_PERIODE_SCRUTATION		(1000)
#define LEDS_PERIODE					(100)

#define XBEE_TX_SEMAPHORE_WAIT		500
#define XBEE_RX_BUFFER_MAX_LENGTH	50

/*
 * Revoir les délais : c'est en 100 ms, pas ms
 */
#define APPLICATION_INACTIVITY_TIMEOUT	(2*60*1000) // 2 minutes, expressed in ms
#define APPLICATION_WATCHDOG_MIN		(900)    	// minimum time to wait before resetting watchdog, expressed in ms
#define APPLICATION_WATCHDOG_MAX		(1100)    	// maximum time to wait before resetting watchdog, expressed in ms
#define APPLICATION_WATCHDOG_MISSED_MAX (3)			// Maximum missed timeout reset before entering watchdog disabled state
#define APPLICATION_STARTUP_DELAY		(3*1000)	// Startup delay, expressed in ms

TickType_t msToTicks(TickType_t ms);
