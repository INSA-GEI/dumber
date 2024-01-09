/**
 ******************************************************************************
 * @file config.h
 * @brief global configuration header
 * @author S. DI MERCURIO (dimercur@insa-toulouse.fr)
 * @date December 2023
 *
 ******************************************************************************
 * @copyright Copyright 2023 INSA-GEI, Toulouse, France. All rights reserved.
 * @copyright This project is released under the Lesser GNU Public License (LGPL-3.0-only).
 *
 * @copyright This file is part of "Dumber" project
 *
 * @copyright This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * @copyright This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * @copyright You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ******************************************************************************
 */

#include "stm32l0xx_hal.h"
#include "cmsis_os.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup CONFIG
 * Constants for global system configuration.
 * @{
 */

/** @name System version
 *  Version in plain text and as a numeric value
 */
///@{
#define SYSTEM_VERSION_STR			"2.1"
#define SYSTEM_VERSION				0x0201	// Upper byte: major version, lower byte: minor version
///@}

#define STACK_SIZE 					0x100

/** @name Tasks priority constants
 *  Priority is based on configMAX_PRIORITIES which represent highest task priority.
 *  Less priority task is \ref PriorityTestsHandler
 */
///@{
#define PriorityLedsAction 				(configMAX_PRIORITIES -1)
#define PriorityXbeeRX 					(configMAX_PRIORITIES -2)
#define PriorityXbeeTX 					(configMAX_PRIORITIES -3)
#define PriorityLedsHandler				(configMAX_PRIORITIES -4)
#define PriorityMotorsAsservissement	(configMAX_PRIORITIES -5)
#define PriorityMotorsHandler			(configMAX_PRIORITIES -6)
#define PriorityBatteryHandler			(configMAX_PRIORITIES -7)
#define PriorityApplicationHandler		(configMAX_PRIORITIES -8)
#define PrioritySequenceurTimeout		(configMAX_PRIORITIES -9)

#define PriorityTestsHandler			(configMAX_PRIORITIES -10)
///@}

/** @name Periodicities of tasks
 *  Periodicities are given in ms.
 */
///@{
#define MOTORS_REGULATION_DELAY			(2)
#define APPLICATION_COUNTERS_DELAY		(100)
#define BATTERY_POLLING_DELAY			(1000)
#define LEDS_DELAY						(100)
#define BUTTON_INACTIVITY_DELAY			(1500)

#define TESTS_DELAY						(5000)
///@}

/** @name Constants for to XBEE module
 *  Constants about timeout and buffer lengths
 */
///@{
#define XBEE_TX_SEMAPHORE_WAIT			500
#ifdef TESTS
#define XBEE_RX_BUFFER_MAX_LENGTH		100
#define XBEE_TX_BUFFER_MAX_LENGTH		100
#else
#define XBEE_RX_BUFFER_MAX_LENGTH		50
#define XBEE_TX_BUFFER_MAX_LENGTH		50
#endif /* TESTS */
///@}

/** @name Constants relative to timeout
 *  Delays are given in ms.
 */
///@{
#ifdef TESTS
#define APPLICATION_INACTIVITY_TIMEOUT	(0xFFFFFFFF) // Max U32, infinite timeout
#else
#define APPLICATION_INACTIVITY_TIMEOUT	(2*60*1000) // 2 minutes, expressed in ms
#endif /* TESTS */

#define APPLICATION_WATCHDOG_MIN		(900)    	// minimum time to wait before resetting watchdog, expressed in ms
#define APPLICATION_WATCHDOG_MAX		(1100)    	// maximum time to wait before resetting watchdog, expressed in ms
#define APPLICATION_WATCHDOG_MISSED_MAX (3)			// Maximum missed timeout reset before entering watchdog disabled state
#define APPLICATION_STARTUP_DELAY		(3*1000)	// Startup delay before entering idle state (3s)
///@}

TickType_t msToTicks(TickType_t ms);

/**
  * @}
  */

/**
  * @}
  */
