/**
 ******************************************************************************
 * @file panic.c
 * @brief panic handler body
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

#include "application.h"
#include "timers.h"

#include "panic.h"
#include "leds.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup PANIC
 * Panic module handles non recoverable error and display an error message on leds
 * @{
 */

/** @addtogroup PANIC_Private Private
 * @{
 */

void PANIC_StopTasksAndWait(void);

/** @cond DOXYGEN_IGNORE_REF */
void MOTORS_PowerOff(void);
/** @endcond */

extern TaskHandle_t xHandleLedsHandler;
extern TaskHandle_t xHandleLedsAction;
extern TaskHandle_t xHandleBattery;
extern TimerHandle_t xHandleTimerButton;
extern TaskHandle_t xHandleApplicationMain;
extern TimerHandle_t xHandleTimerTimeout;
extern TaskHandle_t xHandleMotors;
extern TaskHandle_t xHandleMotorsControl;
extern TaskHandle_t xHandleXbeeTXHandler;
extern TaskHandle_t xHandleXbeeRX;

/**
 * @brief  Handle an unrecoverable error and display corresponding error on leds
 *
 * @param[in] panicId Panic error (as found in \ref PANIC_Typedef)
 * @return None
 *
 * @remark This function will never return (as it calls \ref PANIC_StopTasksAndWait).
 */
void PANIC_Raise(PANIC_Typedef panicId) {
	switch (panicId) {
	case panic_adc_err:
		LEDS_Set(leds_error_1);
		break;
	case panic_charger_err:
		LEDS_Set(leds_error_2);
		break;
	case panic_malloc:
		LEDS_Set(leds_error_3);
		break;
	default:
		LEDS_Set(leds_error_5);
		break;
	}

	PANIC_StopTasksAndWait();
}

/**
 * @brief  Stop all task and stop system
 *
 * All tasks are stopped except led animation (otherwise animation will stop)
 * and, at end, system enter power saving mode.
 *
 * @param None
 * @return None
 *
 * @remark This function will never return (as it calls \ref PANIC_StopTasksAndWait).
 */
void PANIC_StopTasksAndWait(void){
	TaskHandle_t currentTask;
	currentTask = xTaskGetCurrentTaskHandle();

	/* Arret des timers */
	//xTimerStop(xHandleTimerButton,0);
	//xTimerStop(xHandleTimerTimeout,0);

	/* Arret des taches
	 * On n'arrete toute les taches sauf celle en cours !
	 *
	 * a la fin, on arrete la tache en cours
	 *
	 * on n'arrete jamais les leds, sinon plus d'animation !!
	 */

	if (currentTask != xHandleXbeeRX)
		vTaskSuspend(xHandleXbeeRX);

	if (currentTask != xHandleXbeeTXHandler)
			vTaskSuspend(xHandleXbeeTXHandler);

	if (currentTask != xHandleMotors)
			vTaskSuspend(xHandleMotors);

	if (currentTask != xHandleMotorsControl)
			vTaskSuspend(xHandleMotorsControl);

	if (currentTask != xHandleBattery)
			vTaskSuspend(xHandleBattery);

	if (currentTask != xHandleApplicationMain)
			vTaskSuspend(xHandleApplicationMain);


	/* Stop des alim moteurs */
	MOTORS_PowerOff();

	/* disable XBEE */
	HAL_GPIO_WritePin(XBEE_RESET_GPIO_Port, XBEE_RESET_Pin, GPIO_PIN_RESET);

	/* Stop la tache courante */
	vTaskSuspend(currentTask);

	while (1) {
		__WFE(); /* Attente infinie */
	}
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
