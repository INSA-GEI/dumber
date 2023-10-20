/*
 * config.h
 *
 *  Created on: 14 sept. 2022
 *      Author: dimercur
 */
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"

// current version is 2.1
#define SYSTEM_VERSION_STR			"2.1"
#define SYSTEM_VERSION				0x0201	// Upper byte: major version, lower byte: minor version

#define STACK_SIZE 					0x100

#define PriorityLedsAction 				(configMAX_PRIORITIES -1)
#define PriorityXbeeRX 					(configMAX_PRIORITIES -2)
#define PriorityXbeeTX 					(configMAX_PRIORITIES -3)
#define PriorityLedsHandler				(configMAX_PRIORITIES -4)
#define PriorityMoteursAsservissement	(configMAX_PRIORITIES -5)
#define PriorityMoteursHandler			(configMAX_PRIORITIES -6)
#define PriorityBatterieHandler			(configMAX_PRIORITIES -7)
#define PriorityApplicationHandler		(configMAX_PRIORITIES -8)
#define PrioritySequenceurTimeout		(configMAX_PRIORITIES -9)


#define PriorityTestsHandler			(configMAX_PRIORITIES -10)

/* Périodes des taches périodiques (en ms) */
#define MOTEURS_PERIODE_ASSERVISSEMENT	(2)
#define APPLICATION_PERIODE				(100)
#define BATTERIE_PERIODE_SCRUTATION		(1000)
#define LEDS_PERIODE					(100)
#define BUTTON_INACTIVITY_PERIODE		(1500)

#define TESTS_PERIODE					(5000)

#define XBEE_TX_SEMAPHORE_WAIT			500
#ifdef TESTS
#define XBEE_RX_BUFFER_MAX_LENGTH		100
#define XBEE_TX_BUFFER_MAX_LENGTH		100
#else
#define XBEE_RX_BUFFER_MAX_LENGTH		50
#define XBEE_TX_BUFFER_MAX_LENGTH		50
#endif /* TESTS */

#define BATTERY_EMPTY_LEVEL				128
#define BATTERY_LOW_LEVEL				140

/*
 * Revoir les délais : c'est en 100 ms, pas ms
 */
#ifdef TESTS
#define APPLICATION_INACTIVITY_TIMEOUT	(0xFFFFFFFF) // Max U32, infinite timeout
#else
#define APPLICATION_INACTIVITY_TIMEOUT	(2*60*1000) // 2 minutes, expressed in ms
#endif /* TESTS */

#define APPLICATION_WATCHDOG_MIN		(900)    	// minimum time to wait before resetting watchdog, expressed in ms
#define APPLICATION_WATCHDOG_MAX		(1100)    	// maximum time to wait before resetting watchdog, expressed in ms
#define APPLICATION_WATCHDOG_MISSED_MAX (3)			// Maximum missed timeout reset before entering watchdog disabled state
#define APPLICATION_STARTUP_DELAY		(3*1000)	// Startup delay, expressed in ms

TickType_t msToTicks(TickType_t ms);
