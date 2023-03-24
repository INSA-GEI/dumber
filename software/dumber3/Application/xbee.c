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

void XBEE_RxThread(void* params);

SemaphoreHandle_t xHandleSemaphoreRX = NULL;
StaticSemaphore_t xSemaphoreRx;

uint8_t* rxBuffer;
uint8_t rxWaitForACK;
uint8_t rxPhase;

#define XBEE_RX_PHASE_HEADER 	1
#define XBEE_RX_PHASE_BODY		2

/****** TX part ******/
SemaphoreHandle_t xHandleSemaphoreTX = NULL;
SemaphoreHandle_t xHandleSemaphoreTX_ACK = NULL;
StaticSemaphore_t xSemaphoreTX;
StaticSemaphore_t xSemaphoreTX_ACK;

uint8_t* txBuffer;

void XBEE_Init(void) {
	xHandleSemaphoreTX = xSemaphoreCreateBinaryStatic( &xSemaphoreTX );
	xHandleSemaphoreTX_ACK = xSemaphoreCreateBinaryStatic( &xSemaphoreTX_ACK );
	xSemaphoreGive(xHandleSemaphoreTX);

	rxBuffer=(uint8_t*)malloc(XBEE_RX_BUFFER_MAX_LENGTH);
	rxWaitForACK = 0;
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
 * Convert escaped data into raw data. Create a new buffer that is returned
 *
 * \param encodedData pointer on encoded data
 * \param encodedLength length of encoded data
 * \param length length of raw data
 * \return new buffer allocated without escaped char
 */
char* XBEE_DecodeWithoutEscapeChar(char* encodedData, int encodedLength, int *length) {
	char* data = (char*)malloc(encodedLength); 	// on prévoit un buffer aussi grand que celui avec caractère d'echappement,
	// au cas où aucun caractère d'echappement ne serait present
	*length = encodedLength; 					// par défaut, on considère que les données brutes ont la même taille que
	// celles avec caractères d'echappement.

	for (char* p=data; p< (data + encodedLength); p++) {
		if (*encodedData == (char)XBEE_API_ESCAPE_CHAR) {
			encodedData++; // on saute le caractere d'echappement
			*p = (char) (*encodedData & 0x20);
		} else
			*p = *encodedData;

		encodedData++;
	}

	return data;
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
char* XBEE_EncodeTransmissionFrame(char* data, int length, uint16_t destination, int* frameLength) {
	uint8_t checksum;
	char* frame;

	int encodedLength;
	char *encodedData = XBEE_EncodeWithEscapeChar(data, length, &encodedLength);

	*frameLength= encodedLength+9;
	frame = (char*)malloc(*frameLength);

	frame[0] = '~';
	frame[1] = (char)((uint16_t)(*frameLength-3)>>8);
	frame[2] = (char)((uint16_t)(*frameLength-3));
	frame[3] = (char)XBEE_TX_16BIT_REQUEST_TYPE;
	frame[4] = 0x1;

	frame[5] = (char)((uint16_t)(destination)>>8);
	frame[6] = (char)((uint16_t)(destination));

	frame[7] = 0x0;

	char* p_data = encodedData;
	for (char *p=&frame[8]; p<&frame[8] + encodedLength; p++, p_data++) {
		*p= *p_data;
	}

	/* calcul du checksum */
	checksum =0;
	for (int i=3; i<*frameLength; i++) {
		checksum += (uint8_t)frame[i];
	}

	frame[*frameLength] = 0xFF-checksum;

	free ((void*)encodedData);

	return frame;
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
	int incomingDataLength = 0;

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

		*incomingFrame = (XBEE_INCOMING_FRAME*) malloc(sizeof(XBEE_INCOMING_FRAME)); /* Allocate a generic frame struct */
		(*incomingFrame)->type = frame_type;

		switch (frame_type) {
		case XBEE_RX_16BIT_PACKET_TYPE:
			/* Get source address */
			(*incomingFrame)->source_addr = (((uint16_t)rawFrame[4])<<8) + (uint16_t)rawFrame[5];
			(*incomingFrame)->data = XBEE_DecodeWithoutEscapeChar(&rawFrame[8], rawFrameLength-6, &incomingDataLength);
			(*incomingFrame)->ack = 0;
			break;

		case XBEE_MODEM_STATUS_TYPE:
			(*incomingFrame)->modem_status = rawFrame[4];
			(*incomingFrame)->data=0x0;
			break;

		case XBEE_TX_STATUS_TYPE:
			(*incomingFrame)->ack = rawFrame[5];
			(*incomingFrame)->data=0x0;
			break;

		case XBEE_EXTENDED_TX_STATUS_TYPE:
			(*incomingFrame)->ack = rawFrame[8];
			(*incomingFrame)->data=0x0;
			break;

		default:
			free (*incomingFrame);
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
 *                             XBEE_TX_ACK_ERROR in case frame was not acknowledge by detination part
 */
int XBEE_SendData(char* data, int length) {
	int data_length;
	BaseType_t state;
	XBEE_INCOMING_FRAME* incomingFrame;

	// Prevents successive calls to overlap
	state = xSemaphoreTake(xHandleSemaphoreTX, pdMS_TO_TICKS(XBEE_TX_SEMAPHORE_WAIT)); // wait max 500 ms (to avoid interlocking)

	if (state != pdFALSE) { /* test semaphore take answer
	                           if answer is false, it means timeout appends
	                           We should probably reset something in "else" branch */

		/* TODO: stuff to do here for converting data into API frame */
		txBuffer = (uint8_t*)XBEE_EncodeWithEscapeChar(data, length, &data_length);

		if (txBuffer!=NULL) {
			if (HAL_UART_Transmit_DMA(&hlpuart1, txBuffer, data_length)!= HAL_OK)
				return XBEE_TX_ERROR;
			else {
				rxWaitForACK = 1; /* wait for TX ack */
				// Wait for ACK frame after TX
				state = xSemaphoreTake(xHandleSemaphoreTX_ACK, pdMS_TO_TICKS(XBEE_TX_SEMAPHORE_WAIT)); // wait max 500 ms (to avoid interlocking)
				if (XBEE_DecodeFrame((char*) rxBuffer, &incomingFrame)!=XBEE_OK)
					return XBEE_TX_ACK_ERROR;
				else {
					if (incomingFrame == 0x0)
						return XBEE_TX_ACK_ERROR;
					else if ((incomingFrame->type != XBEE_TX_STATUS_TYPE) || (incomingFrame->ack != XBEE_TX_STATUS_SUCCESS)) {
						free ((XBEE_INCOMING_FRAME*) incomingFrame);
						return XBEE_TX_ACK_ERROR;
					}
				}
			}
		} else return XBEE_TX_ERROR;
	} else return XBEE_TX_ERROR;

	return length;
}

/**
 * DMA Interrupt request for transmission. Call when transmission is finished
 *
 * \param UartHandle not used
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/* Free transmit buffer */
	free (txBuffer);

	if (rxWaitForACK != 1) /* we are waiting for an acknowledge frame, so do not give semaphore yet */
		xSemaphoreGiveFromISR( xHandleSemaphoreTX, &xHigherPriorityTaskWoken );

	/* If xHigherPriorityTaskWoken was set to true you
	    we should yield.  The actual macro used here is
	    port specific. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
}

/***** Rx Part *****/

/**
 * Reception thread. Wait for incoming frame, process it and send message to application
 *
 * \param params not used
 */
void XBEE_RxThread(void* params) {
	BaseType_t state;
	XBEE_INCOMING_FRAME* incomingFrame;

	rxPhase= XBEE_RX_PHASE_HEADER;
	while (HAL_UART_Receive_DMA(&hlpuart1, rxBuffer, 3)== HAL_OK); // start reception of frame

	// endless task
	while (1) {
		while ((state = xSemaphoreTake(xHandleSemaphoreRX, portMAX_DELAY))==pdTRUE); // wait forever

		/* Process frame */
		if (XBEE_DecodeFrame((char*) rxBuffer, &incomingFrame)==XBEE_OK) // frame is valid
			if (incomingFrame != 0x0)  // frame is valid
				MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_XBEE_CMD, (QueueHandle_t)0x0, (void*)incomingFrame);
	}
}

/**
 * DMA IRQ handler for reception. Receive a complete frame send send event to sending frame in case of acknowledge frame or to receive task otherwise
 *
 * \param UartHandle not used
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint16_t frameLength;

	if (rxPhase == XBEE_RX_PHASE_HEADER) { // we just received header part
		frameLength=XBEE_GetFrameLength(rxBuffer); /* Compute body part of the frame + checksum */

		if (HAL_UART_Receive_DMA(&hlpuart1, rxBuffer+3, frameLength+1)== HAL_OK) {
			/* Reception of body part started successfully */
			rxPhase = XBEE_RX_PHASE_BODY;
		} else {
			/* Failed to start reception of body
			 * Restart reception of header */
			while (HAL_UART_Receive_DMA(&hlpuart1, rxBuffer, 3)!= HAL_OK); // try to receive header
		}
	} else { // we just received body part. Frame is complete
		if (rxWaitForACK) {
			xSemaphoreGiveFromISR( xHandleSemaphoreTX_ACK, &xHigherPriorityTaskWoken); /* Send event to sending function */
			xSemaphoreGiveFromISR( xHandleSemaphoreTX, &xHigherPriorityTaskWoken ); /* Allow new sending data */
		} else {
			xSemaphoreGiveFromISR( xHandleSemaphoreRX, &xHigherPriorityTaskWoken ); /* send event to receive task to process received task */
		}

		rxPhase = XBEE_RX_PHASE_HEADER;
		while (HAL_UART_Receive_DMA(&hlpuart1, rxBuffer, 3)!= HAL_OK); // try to receive header
	}

	if (xHigherPriorityTaskWoken) {
		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
				    should be performed to ensure the interrupt returns directly to the highest
				    priority task.  The macro used for this purpose is dependent on the port in
				    use and may be called portEND_SWITCHING_ISR(). */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
