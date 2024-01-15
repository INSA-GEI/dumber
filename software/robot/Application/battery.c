/**
 ******************************************************************************
 * @file battery.c
 * @brief battery driver body
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

#include "battery.h"
#include "stm32l0xx.h"
#include "stm32l0xx_ll_gpio.h"
#include "timers.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup BATTERY
 * Battery driver is in charge of monitoring battery voltage, checking for state change in charger,
 * sending message when new events or new voltage acquisitions happened and monitor on/off button
 * @{
 */

/** @addtogroup BATTERY_Private Private
 * @{
 */

/** Enumeration class defining ST601E battery charger chip states */
typedef enum {
	CHARGER_NOT_PLUGGED,		/**< Charger is idle, no USB cable is plugged */
	CHARGER_IN_CHARGE,			/**< Charger is currently charging battery, USB cable is plugged */
	CHARGER_CHARGE_COMPLETE,	/**< Battery charging is finished, charge has ended but USB cable is still plugged */
	CHARGER_ERROR				/**< An error occured during battery charging: charge has ended and USB cable is plugged */
} BATTERY_StatusChargerTypedef;

/** Constant used to removed spurious "CHARGER_ERROR" events */
#define BATTERY_MAX_ERROR		3

extern ADC_HandleTypeDef hadc;
uint8_t conversion_complete;
uint16_t adc_raw_value;

StaticTask_t xTaskBattery;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackBattery[ STACK_SIZE ];
TaskHandle_t xHandleBattery = NULL;
TaskHandle_t task_handler;
TaskHandle_t charger_thread_handler;

/* TimerButton sert à attendre ~ 1.5 secondes avant de prendre en compte les IT bouton
 * En effet, au demarrage, le bouton est appuyé pour lancer le systeme. Ceci genere alors une IT bouton,
 * ammenant à envoyer le message MSG_ID_BUTTON_PRESSED, demandant l'arret du systeme
 *
 * De ce fait, avec cette tempo, on s'assure de ne pas prendre en compte les IT dans les 3 premieres secondes.
 */
StaticTimer_t xBufferTimerButton;
TimerHandle_t xHandleTimerButton = NULL;
void vTimerButtonCallback( TimerHandle_t xTimer );
uint8_t BUTTON_Inactivity=1; //start with button on/off inactive

void BATTERY_Thread(void* params);

/**
  * @brief  Function for initializing battery and on/off button monitoring
  *
  * @param  None
  * @retval None
  */
void BATTERY_Init(void) {
	/* Create the task without using any dynamic memory allocation. */
	xHandleBattery = xTaskCreateStatic(
			BATTERY_Thread,       /* Function that implements the task. */
			"BATTERY Task",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityBatteryHandler,/* Priority at which the task is created. */
			xStackBattery,          /* Array to use as the task's stack. */
			&xTaskBattery);  /* Variable to hold the task's data structure. */

	/* Create a one-shot timer without using any dynamic memory allocation. */
	xHandleTimerButton = xTimerCreateStatic(
			"Inactivity Button Timer",
			pdMS_TO_TICKS(BUTTON_INACTIVITY_DELAY),
			pdTRUE,
			( void * ) 0,
			vTimerButtonCallback,
			&xBufferTimerButton);

	xTimerStart(xHandleTimerButton,0 );
	vTaskResume(xHandleBattery);
}

/**
  * @brief  Read charger status pins and return corresponding charger inner state
  *
  * @param  None
  * @retval Charger current state
  */
BATTERY_StatusChargerTypedef BATTERY_GetChargerStatus(void) {
	uint32_t st2 = LL_GPIO_ReadInputPort(CHARGER_ST2_GPIO_Port) & CHARGER_ST2_Pin;
	uint32_t st1 = LL_GPIO_ReadInputPort(CHARGER_ST1_GPIO_Port) & CHARGER_ST1_Pin;

	BATTERY_StatusChargerTypedef status;

	if (st1 && st2)
		status = CHARGER_NOT_PLUGGED;
	else if (st1 && !st2)
		status = CHARGER_CHARGE_COMPLETE;
	else if (!st1 && st2)
		status = CHARGER_IN_CHARGE;
	else /* !st1 && !st2 */
		status = CHARGER_ERROR;

	return status;
}

/**
  * @brief  Start a voltage acquisition and wait for conversion to end
  *
  * @param[out] val Battery voltage (raw adc value)
  * @return
  * - 0 in case of success
  * - -1 if unable to start ADC
  * - -2 if timeout occured waiting for conversion to end
  */
int BATTERY_GetVoltage(uint16_t *val) {
	uint32_t ulNotificationValue;
	conversion_complete = 0;
	adc_raw_value = 0;

	task_handler = xTaskGetCurrentTaskHandle();

	if (HAL_ADC_Start_IT(&hadc) != HAL_OK)
		return -1;

	ulNotificationValue = ulTaskNotifyTake( pdTRUE, pdMS_TO_TICKS(100)); // wait max 100 ms

	if (ulNotificationValue == 1) {
		/* The transmission ended as expected. */
		*val = adc_raw_value;
	} else {
		/* The call to ulTaskNotifyTake() timed out. */
		return -2;
	}

	task_handler = NULL;

	return 0;
}

/*
 * Il faut considerer ces valeurs comme les seuils de baculement dans une categorie
 * ou une autre
 *
 * Seuil :                         critical          low          high
 * Tension batterie:   2.9   critic  3.1      low    3.3    med    3.6    high     4.2
 *
 */
#ifdef TESTS
uint8_t BATTERY_LEVEL_CRITICAL=135;
uint8_t BATTERY_LEVEL_LOW=145;
uint8_t BATTERY_LEVEL_HIGH=155;

uint8_t BATTERY_LEVEL_CHARGE_LOW=150;
uint8_t BATTERY_LEVEL_CHARGE_HIGH=170;

uint8_t BATTERIE_currentValue;
#else
#define BATTERY_LEVEL_CRITICAL  		135
#define BATTERY_LEVEL_LOW       		145
#define BATTERY_LEVEL_HIGH      		155

#define BATTERY_LEVEL_CHARGE_LOW		150
#define BATTERY_LEVEL_CHARGE_HIGH       170
#endif /* TESTS */

/**
  * @brief  Convert battery voltage into several ranges (level) depending on current charger status (charging or not)
  *         and return corresponding message id to send to application.
  *
  * @param[in] voltage Battery voltage (raw adc value)
  * @param[in] charger status
  * @return message id to be sent to application mailbox
  */
uint16_t BATTERY_BatteryLevel(uint8_t voltage, BATTERY_StatusChargerTypedef chargerStatus) {
	uint16_t msgId=0;

#ifdef TESTS
	BATTERY_currentValue=voltage;
#endif /* TESTS */

	switch (chargerStatus) {
	case CHARGER_CHARGE_COMPLETE:
		msgId = MSG_ID_BAT_CHARGE_COMPLETE;
		break;
	case CHARGER_IN_CHARGE:
		if (voltage<=BATTERY_LEVEL_CHARGE_LOW)
			msgId = MSG_ID_BAT_CHARGE_LOW;
		else if (voltage>=BATTERY_LEVEL_CHARGE_HIGH)
			msgId = MSG_ID_BAT_CHARGE_HIGH;
		else
			msgId = MSG_ID_BAT_CHARGE_MED;
		break;
	case CHARGER_NOT_PLUGGED:
		if (voltage<=BATTERY_LEVEL_CRITICAL)
			msgId = MSG_ID_BAT_CRITICAL_LOW;
		else if (voltage<=BATTERY_LEVEL_LOW)
			msgId = MSG_ID_BAT_LOW;
		else if (voltage>=BATTERY_LEVEL_HIGH)
			msgId = MSG_ID_BAT_HIGH;
		else
			msgId = MSG_ID_BAT_MED;
		break;
	default:
		msgId = MSG_ID_BAT_CHARGE_ERR;
	}

	return msgId;
}

/**
  * @brief  Battery main thread. Periodically , check charger status and battery level.
  *         In case of any change (level or charger status), send a message to application mailbox.
  *         Delay is provided by BATTERY_POLLING_DELAY constant (1 second)
  *
  * @param[in] params startup parameters for task (not used)
  * @return None
  */
void BATTERY_Thread(void* params) {
	static uint16_t voltage;
	static uint8_t batteryErrorCnt=0;
	BATTERY_StatusChargerTypedef currentStatus;
	uint16_t messageID;

	TickType_t xLastWakeTime;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	while (1) {
		if (BATTERY_GetVoltage(&voltage) ==0) {
			currentStatus = BATTERY_GetChargerStatus();
			if (currentStatus == CHARGER_ERROR) {
				batteryErrorCnt++;

				if (batteryErrorCnt>=BATTERY_MAX_ERROR)
					MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_CHARGE_ERR, (QueueHandle_t)0x0, (void*)NULL);
			} else {
				messageID = BATTERY_BatteryLevel(voltage, currentStatus);
				MESSAGE_SendMailbox(APPLICATION_Mailbox, messageID, (QueueHandle_t)0x0, (void*)NULL);
			}
#ifdef TESTS
			MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_LEVEL, (QueueHandle_t)0x0, (void*)&voltage);
#endif /* TESTS*/
		} else {
			MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_ADC_ERR, (QueueHandle_t)0x0, (void*)0x0);
		}

		// Wait for the next cycle.
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(BATTERY_POLLING_DELAY));
	}
}

/**
  * @brief  Callback when ADC conversion end. Store converted value
  *
  * @param[in] hadc ADC informations
  * @return None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	adc_raw_value = HAL_ADC_GetValue(hadc);

	if (task_handler != NULL) {
		/* Notify the task that an event has been emitted. */
		vTaskNotifyGiveFromISR(task_handler, &xHigherPriorityTaskWoken );

		/* There are no more eventin progress, so no tasks to notify. */
		task_handler = NULL;

		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
			    should be performed to ensure the interrupt returns directly to the highest
			    priority task.  The macro used for this purpose is dependent on the port in
			    use and may be called portEND_SWITCHING_ISR(). */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

/**
  * @brief  One-shot timer used to avoid on-off button to trigger event at startup
  *
  * When system power-up, user may still press on/off button while corresponding EXTI is enabled, triggering an interruption that
  * will send a power-off event, disabling system. To avoid this behavior, button events are not accounted during the first 1.5s.
  * This timer is used to disable "BUTTON_Inactivity" flag, restoring normal behavior
  * @param  None
  * @retval None
  */
void vTimerButtonCallback( TimerHandle_t xTimer ) {
	BUTTON_Inactivity=0;

	xTimerStop(xHandleTimerButton,0 );
}

/**
  * @brief  Interrupt handler for external IT (raised by on/off button).
  *
  * @param[in] GPIO_Pin GPIO pin number that raised interruption
  * @return None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//	if (GPIO_Pin == USB_SENSE_Pin) { // Le chargeur vient d'etre branché ou debranché
	//		if (HAL_GPIO_ReadPin(USB_SENSE_GPIO_Port, GPIO_Pin)==GPIO_PIN_SET) // le chargeur est branché
	//			MESSAGE_SendMailboxFromISR(APPLICATION_Mailbox, MSG_ID_BAT_CHARGE_ON, (QueueHandle_t)0x0, 0x0, &xHigherPriorityTaskWoken);
	//		else
	//			MESSAGE_SendMailboxFromISR(APPLICATION_Mailbox, MSG_ID_BAT_CHARGE_OFF, (QueueHandle_t)0x0, 0x0, &xHigherPriorityTaskWoken);
	//	}
	//	else

	if (GPIO_Pin == BUTTON_SENSE_Pin) { // on vient d'appuyer sur le bouton on/off
		if (!BUTTON_Inactivity) {
			if (HAL_GPIO_ReadPin(BUTTON_SENSE_GPIO_Port, GPIO_Pin)==GPIO_PIN_RESET)  // GPIOB.3 = 0 => le bouton est appuyé
				MESSAGE_SendMailboxFromISR(APPLICATION_Mailbox, MSG_ID_BUTTON_PRESSED, (QueueHandle_t)0x0, 0x0, &xHigherPriorityTaskWoken);
		}
	}

	if (xHigherPriorityTaskWoken) {
		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
			    should be performed to ensure the interrupt returns directly to the highest
			    priority task.  The macro used for this purpose is dependent on the port in
			    use and may be called portEND_SWITCHING_ISR(). */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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
