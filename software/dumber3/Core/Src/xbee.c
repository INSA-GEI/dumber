/*
 * xbee.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */


#include "xbee.h"

StaticTask_t xTaskXbeeRX;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackXbeeRX[ STACK_SIZE ];
TaskHandle_t xHandleXbeeRX = NULL;

StaticTask_t xTaskXbeeTX;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackXbeeTX[ STACK_SIZE ];
TaskHandle_t xHandleXbeeTX = NULL;

void XBEE_RxThread(void* params);
void XBEE_TxThread(void* params);

void XBEE_Init(void) {
	/* Create the task without using any dynamic memory allocation. */
	xHandleXbeeTX = xTaskCreateStatic(
			XBEE_TxThread,       /* Function that implements the task. */
			"XBEE Tx",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityXbeeTX,/* Priority at which the task is created. */
			xStackXbeeTX,          /* Array to use as the task's stack. */
			&xTaskXbeeTX);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleXbeeTX);

	/* Create the task without using any dynamic memory allocation. */
	xHandleXbeeRX = xTaskCreateStatic(
			XBEE_RxThread,       /* Function that implements the task. */
			"XBEE Rx",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityXbeeRX,/* Priority at which the task is created. */
			xStackXbeeRX,          /* Array to use as the task's stack. */
			&xTaskXbeeRX);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleXbeeRX);
}

void XBEE_TxThread(void* params) {

}

void XBEE_RxThread(void* params) {

}
