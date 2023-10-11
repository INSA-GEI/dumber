/*
 * batterie.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "batterie.h"
#include "stm32l0xx.h"
#include "stm32l0xx_ll_gpio.h"
#include "timers.h"

typedef enum {
	CHARGEUR_NOT_PLUGGED,
	CHARGEUR_IN_CHARGE,
	CHARGEUR_CHARGE_COMPLETE,
	CHARGEUR_ERROR
} BATTERIE_StatusChargerTypedef;

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

/* TimerButton sert à attendre ~ 3secondes avant de prendre en compte les IT bouton
 * En effet, au demarrage, le bouton est appuyé pour lancer le systeme. ceci genere alors une IT bouton,
 * ammenant à envoyer le message MSG_ID_BUTTON_PRESSED, demandant l'arret du systeme
 *
 * De ce fait, avec cette tempo, on s'assure de ne pas prendre en compte les IT dans les 3 premieres secondes.
 */
StaticTimer_t xBufferTimerButton;
TimerHandle_t xHandleTimerButton = NULL;
void vTimerButtonCallback( TimerHandle_t xTimer );
uint8_t BUTTON_Inactivity=1; //start with button on/off inactive

void BATTERIE_VoltageThread(void* params);

void BATTERIE_Init(void) {
	//task_handler = NULL;
	//charger_thread_handler = NULL;

	/* Create the task without using any dynamic memory allocation. */
	xHandleBatterie = xTaskCreateStatic(
			BATTERIE_VoltageThread,       /* Function that implements the task. */
			"BATTERIE Voltage",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityBatterieHandler,/* Priority at which the task is created. */
			xStackBatterie,          /* Array to use as the task's stack. */
			&xTaskBatterie);  /* Variable to hold the task's data structure. */

	/* Create a periodic task without using any dynamic memory allocation. */
	xHandleTimerButton = xTimerCreateStatic(
			"Inactivity Button Timer",
			pdMS_TO_TICKS(BUTTON_INACTIVITY_PERIODE),
			pdTRUE,
			( void * ) 0,
			vTimerButtonCallback,
			&xBufferTimerButton);

	xTimerStart(xHandleTimerButton,0 );
	vTaskResume(xHandleBatterie);
}

/*
 * Lit les pins GPIO
 */
BATTERIE_StatusChargerTypedef BATTERIE_LireStatusChargeur(void) {
	uint32_t st2 = LL_GPIO_ReadInputPort(CHARGER_ST2_GPIO_Port) & CHARGER_ST2_Pin;
	uint32_t st1 = LL_GPIO_ReadInputPort(CHARGER_ST1_GPIO_Port) & CHARGER_ST1_Pin;

	BATTERIE_StatusChargerTypedef status;

	if (st1 && st2)
		status = CHARGEUR_NOT_PLUGGED;
	else if (st1 && !st2)
		status = CHARGEUR_CHARGE_COMPLETE;
	else if (!st1 && st2)
		status = CHARGEUR_IN_CHARGE;
	else /* !st1 && !st2 */
		status = CHARGEUR_ERROR;

	return status;
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

/*
 * Il faut considerer ces valeurs comme les seuils de baculement dans une categorie
 * ou une autre
 *
 * Seuil :                         critical          low          high
 * Tension batterie:   2.9   critic  3.1      low    3.3    med    3.6    high     4.2
 *
 */
#define BATTERIE_LEVEL_CRITICAL  		200
#define BATTERIE_LEVEL_LOW       		300
#define BATTERIE_LEVEL_HIGH      		500

#define BATTERIE_LEVEL_CHARGE_LOW		400
#define BATTERIE_LEVEL_CHARGE_HIGH      700

uint16_t BATTERIE_BatteryLevel(uint8_t voltage, BATTERIE_StatusChargerTypedef chargerStatus) {
	uint16_t msgId=0;

	switch (chargerStatus) {
	case CHARGEUR_CHARGE_COMPLETE:
		msgId = MSG_ID_BAT_CHARGE_COMPLETE;
		break;
	case CHARGEUR_IN_CHARGE:
		if (voltage<=BATTERIE_LEVEL_CHARGE_LOW)
			msgId = MSG_ID_BAT_CHARGE_LOW;
		else if (voltage>=BATTERIE_LEVEL_CHARGE_HIGH)
			msgId = MSG_ID_BAT_CHARGE_HIGH;
		else
			msgId = MSG_ID_BAT_CHARGE_MED;
		break;
	case CHARGEUR_NOT_PLUGGED:
		if (voltage<=BATTERIE_LEVEL_CRITICAL)
			msgId = MSG_ID_BAT_CRITICAL_LOW;
		else if (voltage<=BATTERIE_LEVEL_LOW)
			msgId = MSG_ID_BAT_LOW;
		else if (voltage>=BATTERIE_LEVEL_HIGH)
			msgId = MSG_ID_BAT_HIGH;
		else
			msgId = MSG_ID_BAT_MED;
		break;
	default:
		msgId = MSG_ID_BAT_CHARGE_ERR;
	}

	return msgId;
}

void BATTERIE_VoltageThread(void* params) {
	static uint16_t tension;
	BATTERIE_StatusChargerTypedef currentStatus;
	uint16_t messageID;

	TickType_t xLastWakeTime;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	while (1) {
		if (BATTERIE_LireTension(&tension) ==0) {
			currentStatus = BATTERIE_LireStatusChargeur();
			if (currentStatus == CHARGEUR_ERROR)
				MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_CHARGE_ERR, (QueueHandle_t)0x0, (void*)NULL);
			/*else if (currentStatus == CHARGEUR_IN_CHARGE)
				MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_CHARGE_ON, (QueueHandle_t)0x0, (void*)&tension);
			else if (currentStatus == CHARGEUR_CHARGE_COMPLETE)
				MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_CHARGE_COMPLETE, (QueueHandle_t)0x0, (void*)&tension);
			else
				MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_CHARGE_OFF, (QueueHandle_t)0x0, (void*)&tension);*/
			messageID = BATTERIE_BatteryLevel(tension, currentStatus);
			MESSAGE_SendMailbox(APPLICATION_Mailbox, messageID, (QueueHandle_t)0x0, (void*)NULL);

#ifdef TESTS
			MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_LEVEL, (QueueHandle_t)0x0, (void*)&tension);
#endif /* TESTS*/
		} else {
			MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BAT_ADC_ERR, (QueueHandle_t)0x0, (void*)0x0);
		}

		// Wait for the next cycle.
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(BATTERIE_PERIODE_SCRUTATION));
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

void vTimerButtonCallback( TimerHandle_t xTimer ) {
	BUTTON_Inactivity=0;

	xTimerStop(xHandleTimerButton,0 );
}

/**
 * @brief This function handles EXTI line0 interrupt.
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
