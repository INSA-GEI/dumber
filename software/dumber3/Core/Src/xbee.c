/*
 * xbee.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */


#include "xbee.h"
#include "semphr.h"
#include <stdlib.h>
#include <string.h>

extern UART_HandleTypeDef hlpuart1;
extern DMA_HandleTypeDef hdma_lpuart1_tx;

StaticTask_t xTaskXbeeRX;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackXbeeRX[ STACK_SIZE ];
TaskHandle_t xHandleXbeeRX = NULL;

SemaphoreHandle_t xSemaphoreTX = NULL;
StaticSemaphore_t xSemaphoreTXBuffer;

uint8_t* txBuffer;

uint8_t* rxBuffer;
uint32_t rxBufferIndex;

void XBEE_RxThread(void* params);

void XBEE_Init(void) {
	xSemaphoreTX = xSemaphoreCreateBinaryStatic( &xSemaphoreTXBuffer );
	xSemaphoreGive(xSemaphoreTX);

	rxBuffer=(uint8_t*)malloc(XBEE_RX_BUFFER_MAX_LENGTH);
	rxBufferIndex=0;

	if(HAL_UART_Receive_IT(&hlpuart1, rxBuffer, 1)!= HAL_OK) {
		while(1);
	}
}

int XBEE_SendData(char* data, int length) {
	int data_length = length;
	BaseType_t state;

	state = xSemaphoreTake(xSemaphoreTX, pdMS_TO_TICKS(XBEE_TX_SEMAPHORE_WAIT)); // wait max 500 ms

	if (state != pdFALSE) { /* test semaphore take answer */
		if ((txBuffer = (uint8_t*)malloc(length))!=NULL) {
			memcpy((void*)txBuffer, (void*) data, length);

			if(HAL_UART_Transmit_IT(&hlpuart1, txBuffer, data_length)!= HAL_OK)
				return -3;
		} else return -2;
	} else return -1;

	return length;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/* Do stuff here */
	free (txBuffer);

	xSemaphoreGiveFromISR( xSemaphoreTX, &xHigherPriorityTaskWoken );

	/* If xHigherPriorityTaskWoken was set to true you
	    we should yield.  The actual macro used here is
	    port specific. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (rxBuffer[rxBufferIndex] == '\r') {
		rxBuffer[rxBufferIndex+1]=0; // add end of string
		MESSAGE_SendMailboxFromISR(SEQUENCEUR_Mailbox, MSG_ID_XBEE_CMD, (QueueHandle_t)0x0, (void*)rxBuffer, &xHigherPriorityTaskWoken);

		rxBuffer = malloc(XBEE_RX_BUFFER_MAX_LENGTH);
		rxBufferIndex=0;
	} else if (rxBufferIndex>=XBEE_RX_BUFFER_MAX_LENGTH-2) // prevent buffer overflow
		rxBufferIndex=0;
	else
		rxBufferIndex++;

	if(HAL_UART_Receive_IT(&hlpuart1, &rxBuffer[rxBufferIndex], 1)!= HAL_OK) {
		while(1);
	}

	if (xHigherPriorityTaskWoken) {
		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
				    should be performed to ensure the interrupt returns directly to the highest
				    priority task.  The macro used for this purpose is dependent on the port in
				    use and may be called portEND_SWITCHING_ISR(). */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
