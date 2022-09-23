/*
 * statemachine.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "sequenceur.h"

StaticTask_t xTaskSequenceurMain;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackSequenceurMain[ STACK_SIZE ];
TaskHandle_t xHandleSequenceurMain = NULL;

StaticTask_t xTaskSequenceurTimeout;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackSequenceurTimeout[ STACK_SIZE ];
TaskHandle_t xHandleSequenceurTimeout = NULL;

void SEQUENCEUR_MainThread(void* params);
void SEQUENCEUR_TimeoutThread(void* params);

uint16_t SEQUENCEUR_CntTimeout;
uint16_t SEQUENCEUR_CntPowerOff;

void SEQUENCEUR_Init(void) {
	/* Create the task without using any dynamic memory allocation. */
	xHandleSequenceurMain = xTaskCreateStatic(
			SEQUENCEUR_MainThread,       /* Function that implements the task. */
			"SEQUENCEUR Main",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PrioritySequenceurMain,/* Priority at which the task is created. */
			xStackSequenceurMain,          /* Array to use as the task's stack. */
			&xTaskSequenceurMain);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleSequenceurMain);

	/* Create the task without using any dynamic memory allocation. */
	xHandleSequenceurTimeout = xTaskCreateStatic(
			SEQUENCEUR_TimeoutThread,       /* Function that implements the task. */
			"SEQUENCEUR Timeout",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PrioritySequenceurTimeout,/* Priority at which the task is created. */
			xStackSequenceurTimeout,          /* Array to use as the task's stack. */
			&xTaskSequenceurTimeout);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleSequenceurTimeout);

	SEQUENCEUR_CntTimeout =0;
	SEQUENCEUR_CntPowerOff=0;
}

void SEQUENCEUR_MainThread(void* params) {

}

void SEQUENCEUR_TimeoutThread(void* params) {

}
