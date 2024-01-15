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
#include "stm32l0xx_ll_usart.h"

extern UART_HandleTypeDef hlpuart1;
extern DMA_HandleTypeDef hdma_lpuart1_tx;
extern DMA_HandleTypeDef hdma_lpuart1_rx;

/***** API2 escaped char *****/
#define XBEE_API_ESCAPE_CHAR	0x7D
#define XBEE_API_START_OF_FRAME 0x7E
#define XBEE_API_XON			0x11
#define XBEE_API_XOFF			0x13

/***** RX part *****/

/* Stuff for Xbee RX task */
StaticTask_t xTaskXbeeRX;
StackType_t xStackXbeeRX[ STACK_SIZE ];
TaskHandle_t xHandleXbeeRX = NULL;

uint8_t txBuffer[XBEE_RX_BUFFER_MAX_LENGTH]={0};
uint16_t txIndex;
uint16_t txDataToSend;

void XBEE_RxThread(void* params);

SemaphoreHandle_t xHandleSemaphoreRX = NULL;
StaticSemaphore_t xSemaphoreRx;

uint8_t rxBuffer[XBEE_RX_BUFFER_MAX_LENGTH]={0};
//uint8_t rxWaitForACK =0;
uint8_t rxPhase;
uint16_t rxFrameLength;
uint16_t rxDataToReceive;
uint16_t rxIndex;

#define XBEE_RX_PHASE_SOF		0
#define XBEE_RX_PHASE_HEADER 	1
#define XBEE_RX_PHASE_BODY		2

/****** TX part ******/
SemaphoreHandle_t xHandleSemaphoreTX = NULL;
SemaphoreHandle_t xHandleSemaphoreTX_ACK = NULL;
StaticSemaphore_t xSemaphoreTX;
StaticSemaphore_t xSemaphoreTX_ACK;

void XBEE_Init(void) {
	xHandleSemaphoreTX = xSemaphoreCreateBinaryStatic( &xSemaphoreTX );
	xHandleSemaphoreTX_ACK = xSemaphoreCreateBinaryStatic( &xSemaphoreTX_ACK );
	xSemaphoreGive(xHandleSemaphoreTX);
	//xSemaphoreTake(xHandleSemaphoreTX_ACK);

	xHandleSemaphoreRX = xSemaphoreCreateBinaryStatic( &xSemaphoreRx );

	/* Create the task without using any dynamic memory allocation. */
	xHandleXbeeRX = xTaskCreateStatic(
			XBEE_RxThread,       /* Function that implements the task. */
			"XBEE Rx",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityMoteurs,/* Priority at which the task is created. */
			xStackXbeeRX,          /* Array to use as the task's stack. */
			&xTaskXbeeRX);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleXbeeRX);

	/* Enable Xbee */
	HAL_GPIO_WritePin(XBEE_RESET_GPIO_Port, XBEE_RESET_Pin, GPIO_PIN_SET);
}

/**** Support functions ****/

/**
 * Compute length of encoded data using escape char
 *
 * \param data pointer on raw data
 * \param length length of raw data
 * \return length of data when encoded
 */
int XBEE_GetDataLength(char* data, int length) {
	int encodedLength=0;

	for (int i=0; i< length; i++) {
		if ((*data == (char)XBEE_API_ESCAPE_CHAR) || (*data == (char)XBEE_API_START_OF_FRAME) ||
				(*data == (char)XBEE_API_XOFF) ||(*data == (char)XBEE_API_XON)) {
			encodedLength++; /* Add 1 byte for escape char */
		}

		encodedLength++; /* Add current char in length calculation */
		data++; /* move to next byte in raw buffer */
	}

	return encodedLength;
}

/**
 * Convert raw data into escaped data. Create a new buffer that is returned
 *
 * \param data pointer on raw data
 * \param length length of raw data
 * \param encodedLength length of encoded data
 * \return new buffer allocated with escaped char
 */
char* XBEE_EncodeWithEscapeChar(char* data, int length, int *encodedLength) {
	char* encodedData;

	*encodedLength=XBEE_GetDataLength(data, length);
	encodedData = (char*)malloc(*encodedLength);

	for (char* p=encodedData; p< (encodedData + *encodedLength); p++) {
		if ((*data == (char)XBEE_API_ESCAPE_CHAR) || (*data == (char)XBEE_API_START_OF_FRAME) ||
				(*data == (char)XBEE_API_XOFF) ||(*data == (char)XBEE_API_XON)) {
			*p = (char) XBEE_API_ESCAPE_CHAR;
			p++;
		}

		*p = *data;
		data++;
	}

	return encodedData;
}

/**
 * Get a raw buffer and convert it into a transmission frame
 *
 * \param data pointer on raw data
 * \param length length of raw data
 * \param destination address of the destination
 * \param frameLength length of frame
 * \return new frame (allocated) containing escaped data
 */
void XBEE_EncodeTransmissionFrame(char* data, int length, uint16_t destination, char* frame, int* frameLength) {
	uint8_t checksum=0;
	uint16_t localLength=0;
	char* p;

	localLength = length+9;

	frame[0] = (char)XBEE_FRAME_SOF_CHAR;
	frame[1] = (char)((uint16_t)(localLength-4)>>8);
	frame[2] = (char)((uint16_t)(localLength-4));
	frame[3] = (char)XBEE_TX_16BIT_REQUEST_TYPE;
	frame[4] = 0x1;

	frame[5] = (char)((uint16_t)(destination)>>8);
	frame[6] = (char)((uint16_t)(destination));

	frame[7] = 0x0;

	for (p = &frame[8]; p< (data + length); p++) {
		if ((*data == (char)XBEE_API_ESCAPE_CHAR) || (*data == (char)XBEE_API_START_OF_FRAME) ||
				(*data == (char)XBEE_API_XOFF) ||(*data == (char)XBEE_API_XON)) {
			*p = (char) XBEE_API_ESCAPE_CHAR;
			p++;
			*p = *data^0x20;
		} else
			*p = *data;

		data++;
	}

	/* calcul du checksum */
	for (int i=3; i<(localLength-1); i++) {
		checksum += (uint8_t)frame[i];
	}

	*p = 0xFF-checksum;
	*frameLength = localLength;
}

/**
 * Get frame length
 *
 * \param frame pointer on frame header
 * \return length of incoming frame
 */
uint16_t XBEE_GetFrameLength(uint8_t *frame) {
	return (((uint16_t)frame[1])<<8) + (uint16_t)frame[2];
}

/**
 * Get a complete frame, check if frame is correct and extract raw data
 *
 * \param raw_frame pointer on complete frame
 * \param incomingFrame pointer to processed frame, with escape char removed
 * \return status of decoding: XBEE_OK if decoding is successful, XBEE_INVALID_FRAME otherwise
 */
XBEE_Status XBEE_DecodeFrame(char* rawFrame, XBEE_INCOMING_FRAME** incomingFrame) {
	uint8_t frame_type = (uint8_t)rawFrame[3];
	uint16_t rawFrameLength;
	uint8_t checksum;
	XBEE_Status status = XBEE_OK;
	int allocatedSize;
	int dataSize;

	int i;

	if (rawFrame[0] == '~') {
		rawFrameLength = (((uint16_t)rawFrame[1])<<8) + (uint16_t)rawFrame[2];

		/* verification du checksum */
		checksum =0;
		for (i=3; i<3+rawFrameLength+1; i++) {
			checksum += (uint8_t)rawFrame[i];
		}

		if (checksum != 0xFF)
			return XBEE_INVALID_FRAME;

		switch (frame_type) {
		case XBEE_RX_16BIT_PACKET_TYPE:
			dataSize = rawFrameLength-5; // there is 5 bytes of "other" data than truly data bytes in a frame
			allocatedSize = sizeof(XBEE_INCOMING_FRAME)+dataSize;

			*incomingFrame = (XBEE_INCOMING_FRAME*) malloc(allocatedSize); /* Allocate a generic frame struct */
			(*incomingFrame)->type = frame_type;

			/* Get source address */
			(*incomingFrame)->source_addr = (((uint16_t)rawFrame[4])<<8) + (uint16_t)rawFrame[5];
			//XBEE_DecodeWithoutEscapeChar(&rawFrame[8], rawFrameLength-5, (*incomingFrame)->data, &incomingDataLength); // Data = Frame length -5
			(*incomingFrame)->length = (uint8_t)(dataSize);
			(*incomingFrame)->ack = 0;

			for (i=0; i<dataSize; i++)
				(*incomingFrame)->data[i] = rawFrame[i+8];

			break;

		case XBEE_MODEM_STATUS_TYPE:
			allocatedSize = sizeof(XBEE_INCOMING_FRAME); // no data
			*incomingFrame = (XBEE_INCOMING_FRAME*) malloc(allocatedSize); /* Allocate a generic frame struct */

			(*incomingFrame)->type = frame_type;

			(*incomingFrame)->modem_status = rawFrame[4];
			(*incomingFrame)->data[0]=0x0;
			(*incomingFrame)->length = 0;
			break;

		case XBEE_TX_STATUS_TYPE:
			allocatedSize = sizeof(XBEE_INCOMING_FRAME); // no data
			*incomingFrame = (XBEE_INCOMING_FRAME*) malloc(allocatedSize); /* Allocate a generic frame struct */

			(*incomingFrame)->ack = rawFrame[5];
			(*incomingFrame)->data[0]=0x0;
			(*incomingFrame)->length = 0;
			break;

		case XBEE_EXTENDED_TX_STATUS_TYPE:
			allocatedSize = sizeof(XBEE_INCOMING_FRAME); // no data
			*incomingFrame = (XBEE_INCOMING_FRAME*) malloc(allocatedSize); /* Allocate a generic frame struct */

			(*incomingFrame)->ack = rawFrame[8];
			(*incomingFrame)->data[0]=0x0;
			(*incomingFrame)->length = 0;
			break;

		default:
			*incomingFrame=NULL;
			return XBEE_INVALID_FRAME;
		};
	} else status = XBEE_INVALID_FRAME;

	return status;
}

/**** TX Part *****/

/**
 * Send data. Create a transmission frame, add escape char to data and send it over UART
 *
 * \param data raw data to send
 * \param length length of data to send
 * \return status of decoding: XBEE_OK if decoding is successful,
 *                             XBEE_TX_ERROR in case of sending error,
 *                             XBEE_TX_TIMEOUT in case semaphore takes too long
 */
int XBEE_SendData(uint16_t address, char* data, int length) {
	BaseType_t state;
	int status = XBEE_OK;

	// Prevents successive calls to overlap
	state = xSemaphoreTake(xHandleSemaphoreTX, pdMS_TO_TICKS(XBEE_TX_SEMAPHORE_WAIT)); // wait max 500 ms (to avoid interlocking)

	if (state != pdFALSE) { /* test semaphore take answer
	                           if answer is false, it means timeout appends
	                           We should probably reset something in "else" branch */

		XBEE_EncodeTransmissionFrame(data, length, address, (char*) txBuffer, (int*)&txDataToSend);

		LL_USART_TransmitData8(hlpuart1.Instance, txBuffer[0]);
		txIndex =1;
		LL_USART_EnableIT_TXE(hlpuart1.Instance); // enable TX Interrupt
	} else status= XBEE_TX_TIMEOUT;

	return status;
}

void XBEE_TX_IRQHandler(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	LL_USART_TransmitData8(hlpuart1.Instance, txBuffer[txIndex]);
	txIndex++;
	if (txIndex == txDataToSend) {
		LL_USART_DisableIT_TXE(hlpuart1.Instance);
		xSemaphoreGiveFromISR( xHandleSemaphoreTX, &xHigherPriorityTaskWoken );
	}

	if (xHigherPriorityTaskWoken) {
		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
					    should be performed to ensure the interrupt returns directly to the highest
					    priority task.  The macro used for this purpose is dependent on the port in
					    use and may be called portEND_SWITCHING_ISR(). */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

/***** Rx Part *****/

/**
 * Reception thread. Wait for incoming frame, process it and send message to application
 *
 * \param params not used
 */
void XBEE_RxThread(void* params) {
	XBEE_INCOMING_FRAME *incomingFrame;

	rxPhase= XBEE_RX_PHASE_SOF;
	rxFrameLength=0;
	rxDataToReceive=1;
	rxIndex=0;

	while (HAL_UART_Receive_IT(&hlpuart1, rxBuffer, 1)!= HAL_OK); // try starting reception of frame
	LL_USART_Disable(hlpuart1.Instance);
	LL_USART_DisableOverrunDetect(hlpuart1.Instance);
	LL_USART_Enable(hlpuart1.Instance);

	// endless task
	while (1) {
		if (xSemaphoreTake(xHandleSemaphoreRX, portMAX_DELAY)==pdTRUE) { // wait forever

			/* Process frame */
			if (XBEE_DecodeFrame((char*) rxBuffer, &incomingFrame)==XBEE_OK) { // frame is valid
				if (incomingFrame != 0x0)  // frame is valid
					MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_XBEE_CMD, (QueueHandle_t)0x0, (void*)incomingFrame);
				//			if (rxBuffer[3]== XBEE_RX_16BIT_PACKET_TYPE) {
				//				MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_XBEE_CMD, (QueueHandle_t)0x0, (void*)0x0);
			}

			for (int i=0; i<XBEE_RX_BUFFER_MAX_LENGTH; i++)
				rxBuffer[i]=0xFF;
		}
	}
}

void XBEE_RX_IRQHandler(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint8_t data;

	data = LL_USART_ReceiveData8(hlpuart1.Instance); // lecture de l'octet reçu

	switch (rxPhase) {
	case XBEE_RX_PHASE_SOF:
		if (data == XBEE_FRAME_SOF_CHAR) {
			rxBuffer[0] = data;
			rxPhase = XBEE_RX_PHASE_HEADER;
			rxIndex = 1;
		}
		break;
	case XBEE_RX_PHASE_HEADER:
		rxBuffer[rxIndex] = data;
		rxIndex ++;

		if (rxIndex==3) {
			// header reçu, calcul de la longueur de la frame
			rxFrameLength = (((uint16_t)rxBuffer[1]) << 8) + (uint16_t)rxBuffer[2];
			rxDataToReceive = rxFrameLength +1; // +1 for checksum
			rxPhase = XBEE_RX_PHASE_BODY;
		}
		break;
	case XBEE_RX_PHASE_BODY:
		rxBuffer[rxIndex] = data;
		if (rxBuffer[rxIndex-1] == XBEE_API_ESCAPE_CHAR)
			rxBuffer[rxIndex-1] = rxBuffer[rxIndex] ^ 0x20;
		else {
			rxIndex++;
			rxDataToReceive--;
		}

		if (!rxDataToReceive) {
			rxIndex=0;
			rxPhase = XBEE_RX_PHASE_SOF;
			xSemaphoreGiveFromISR( xHandleSemaphoreRX, &xHigherPriorityTaskWoken ); /* send event to receive task to process received task */
		}
		break;
	default:
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

/**
 * DMA IRQ handler for reception. Receive a complete frame send send event to sending frame in case of acknowledge frame or to receive task otherwise
 *
 * \param UartHandle not used
 */
void LPUART1_IRQHandler(void) {

	if (LL_USART_IsActiveFlag_RXNE(hlpuart1.Instance)) {
		XBEE_RX_IRQHandler();
	} else if (LL_USART_IsActiveFlag_TXE(hlpuart1.Instance)) {
		XBEE_TX_IRQHandler();
	} else {
		if (LL_USART_IsActiveFlag_TC(hlpuart1.Instance))
			LL_USART_DisableIT_TC(hlpuart1.Instance);
		else if (LL_USART_IsActiveFlag_IDLE(hlpuart1.Instance))
			LL_USART_ClearFlag_IDLE(hlpuart1.Instance);
		else if (LL_USART_IsActiveFlag_ORE(hlpuart1.Instance)) {
			LL_USART_ClearFlag_ORE(hlpuart1.Instance);
		}
	}
}
