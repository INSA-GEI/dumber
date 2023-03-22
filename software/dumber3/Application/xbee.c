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
uint32_t rxBufferIndex;

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
	rxBufferIndex=0;

	if(HAL_UART_Receive_IT(&hlpuart1, rxBuffer, 1)!= HAL_OK) {
		while(1);
	}
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
	char* data = (char*)malloc(encodedLength); 	// on prevoit un buffer aussi grand que celui avec caractere d'echappement,
												// au cas où aucun caractere d'echappement ne serait present
	*length = encodedLength; 					// par defaut, on considére que les données brutes ont la même taille que
												// celles avec caractéres d'echappement.

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
 * Get a complete frame, check if frame is correct and extract raw data
 *
 * \param frame pointer on complete frame
 * \param data pointer to raw data, with escape char removed
 * \param sender address of the sender
 * \return status of decoding: XBEE_OK if decoding is successfull, XBEE_INVALID_FRAME otherwise
 */
XBEE_Status XBEE_DecodeFrame(char* frame, char** data, uint16_t *sender) {
	uint8_t frame_type = (uint8_t)frame[3];
	uint16_t frame_length;
	uint8_t checksum;
	int i;

	frame_length = (((uint16_t)frame[1])<<8) + (uint16_t)frame[2];

	/* verification du checksum */
	checksum =0;
	for (i=3; i<3+frame_length+1; i++) {
		checksum += (uint8_t)frame[i];
	}

	if (checksum != 0xFF)
		return XBEE_INVALID_FRAME;

	switch (frame_type) {
	case XBEE_RX_PACKET_TYPE:
		*data = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_RX_PACKET_FRAME)+(frame_length-12)+1); // +1 for 0 ending in data frame
		memset((void*)*data, 0, sizeof(XBEE_RX_PACKET_FRAME)+(frame_length-12)+1);
		((XBEE_RX_PACKET_FRAME*)(*data))->type = frame_type;
		((XBEE_RX_PACKET_FRAME*)(*data))->data_length = frame_length-12;
		((XBEE_RX_PACKET_FRAME*)(*data))->options = frame[14];

		((XBEE_RX_PACKET_FRAME*)(*data))->source_addr =0;
		for (i=0; i<8; i++) {
			((XBEE_RX_PACKET_FRAME*)(*data))->source_addr = ((XBEE_RX_PACKET_FRAME*)(*data))->source_addr<<8;
			((XBEE_RX_PACKET_FRAME*)(*data))->source_addr +=(uint64_t)frame[4+i];
		}

		for (i=0; i<((XBEE_RX_PACKET_FRAME*)(*data))->data_length; i++) {
			((XBEE_RX_PACKET_FRAME*)(*data))->data[i] =(char)frame[15+i];
		}

		((XBEE_RX_PACKET_FRAME*)(*data))->data[((XBEE_RX_PACKET_FRAME*)(*data))->data_length]=0x0; // 0 ending frame

		break;
	case XBEE_MODEM_STATUS_TYPE:
		*data = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_MODEM_STATUS_FRAME));
		((XBEE_MODEM_STATUS_FRAME*)(*data))->type = frame_type;
		((XBEE_MODEM_STATUS_FRAME*)(*data))->status = frame[4];

		break;
	case XBEE_TX_STATUS_TYPE:
	case XBEE_EXTENDED_TX_STATUS_TYPE:
		*data = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_TX_STATUS_FRAME));
		((XBEE_TX_STATUS_FRAME*)(*data))->type = frame_type;
		((XBEE_TX_STATUS_FRAME*)(*data))->frame_id = frame[4];
		if (frame_type == XBEE_TX_STATUS_TYPE) {
			((XBEE_TX_STATUS_FRAME*)(*data))->status = frame[5];
			((XBEE_TX_STATUS_FRAME*)(*data))->retry_count = 0;
		} else {
			((XBEE_TX_STATUS_FRAME*)(*data))->status = frame[8];
			((XBEE_TX_STATUS_FRAME*)(*data))->retry_count = frame[7];
		}

		break;
	case XBEE_AT_CMD_RESPONSE_TYPE:
		*data = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_AT_CMD_RESPONSE_FRAME)+(frame_length-5));
		((XBEE_AT_CMD_RESPONSE_FRAME*)(*data))->type = frame_type;
		((XBEE_AT_CMD_RESPONSE_FRAME*)(*data))->data_length = frame_length-5;
		((XBEE_AT_CMD_RESPONSE_FRAME*)(*data))->status = frame[7];

		for (i=0; i<((XBEE_AT_CMD_RESPONSE_FRAME*)(*data))->data_length; i++) {
			((XBEE_AT_CMD_RESPONSE_FRAME*)(*data))->data[i] =(uint8_t)frame[8+i];
		}
		break;
	default:
		*data = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_GENERIC_FRAME));
		((XBEE_GENERIC_FRAME*)(*data))->type = frame_type;
		return XBEE_INVALID_FRAME;
	};

	return XBEE_OK;
}

/**** TX Part *****/
int XBEE_SendData(char* data, int length) {
	int data_length;
	BaseType_t state;

	// Prevents successive calls to overlap
	state = xSemaphoreTake(xHandleSemaphoreTX, pdMS_TO_TICKS(XBEE_TX_SEMAPHORE_WAIT)); // wait max 500 ms (to avoid interlocking)

	if (state != pdFALSE) { /* test semaphore take answer
	                           if answer is false, it means timeout appends
	                           We should probably reset something in "else" branch */

		/* TODO: stuff to do here for converting data into API frame */
		data_length = length;
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

	xSemaphoreGiveFromISR( xHandleSemaphoreTX, &xHigherPriorityTaskWoken );

	/* If xHigherPriorityTaskWoken was set to true you
	    we should yield.  The actual macro used here is
	    port specific. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
}

/***** Rx Part *****/
void XBEE_RxThread(void* params) {

	// endless task
	while (1) {

	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (rxBuffer[rxBufferIndex] == '\r') {
		rxBuffer[rxBufferIndex+1]=0; // add end of string
		MESSAGE_SendMailboxFromISR(APPLICATION_Mailbox, MSG_ID_XBEE_CMD, (QueueHandle_t)0x0, (void*)rxBuffer, &xHigherPriorityTaskWoken);

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
