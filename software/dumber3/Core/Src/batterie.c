/*
 * batterie.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "main.h"

extern ADC_HandleTypeDef hadc;
uint8_t conversion_complete;
uint16_t adc_raw_value;

StaticTask_t xTaskBatterie;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackBatterie[ STACK_SIZE ];
TaskHandle_t xHandleBatterie = NULL;
TaskHandle_t task_handler;
TaskHandle_t charger_thread_handler;

void BATTERIE_VoltageThread(void* params);

void BATTERIE_Init(void) {
	task_handler = NULL;
	charger_thread_handler = NULL;

	/* Create the task without using any dynamic memory allocation. */
	xHandleBatterie = xTaskCreateStatic(
			BATTERIE_VoltageThread,       /* Function that implements the task. */
			"BATTERIE Voltage",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityLeds,/* Priority at which the task is created. */
			xStackBatterie,          /* Array to use as the task's stack. */
			&xTaskBatterie);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleBatterie);
}

int BATTERIE_LireTension(uint16_t *val) {
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

void BATTERIE_VoltageThread(void* params) {
	static uint16_t tension;

	while (1) {
		if (BATTERIE_LireTension(&tension) ==0) {
			if (HAL_GPIO_ReadPin(GPIOB, USB_SENSE_Pin)==GPIO_PIN_SET) // le chargeur est branché
				MESSAGE_SendMailbox(SEQUENCEUR_Mailbox, MSG_ID_BAT_CHARGE, (QueueHandle_t)0x0, (void*)&tension);
			else
				MESSAGE_SendMailbox(SEQUENCEUR_Mailbox, MSG_ID_BAT_NIVEAU, (QueueHandle_t)0x0, (void*)&tension);
		} else {
			MESSAGE_SendMailbox(SEQUENCEUR_Mailbox, MSG_ID_BAT_ADC_ERR, (QueueHandle_t)0x0, (void*)0x0);
		}

		vTaskDelay(pdMS_TO_TICKS(BATTERIE_PERIODE_SCRUTATION));
	}
}

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
 * @brief This function handles EXTI line0 interrupt.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (GPIO_Pin == USB_SENSE_Pin) { // Le chargeur vient d'etre branché ou debranché
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_Pin)==GPIO_PIN_SET) // le chargeur est branché
			MESSAGE_SendMailboxFromISR(SEQUENCEUR_Mailbox, MSG_ID_BAT_CHARGEUR_ON, (QueueHandle_t)0x0, 0x0, &xHigherPriorityTaskWoken);
		else
			MESSAGE_SendMailboxFromISR(SEQUENCEUR_Mailbox, MSG_ID_BAT_CHARGEUR_OFF, (QueueHandle_t)0x0, 0x0, &xHigherPriorityTaskWoken);
	}
	else if (GPIO_Pin == BUTTON_SENSE_Pin) { // on vient d'appuyer sur le bouton on/off
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_Pin)==GPIO_PIN_SET)  // le chargeur est branché
			MESSAGE_SendMailboxFromISR(SEQUENCEUR_Mailbox, MSG_ID_BUTTON_PRESSED, (QueueHandle_t)0x0, 0x0, &xHigherPriorityTaskWoken);
	}

	if (xHigherPriorityTaskWoken) {
		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
			    should be performed to ensure the interrupt returns directly to the highest
			    priority task.  The macro used for this purpose is dependent on the port in
			    use and may be called portEND_SWITCHING_ISR(). */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
