/*
 * xbee.c
 *
 *  Created on: 4 avr. 2022
 *      Author: dimercur
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "xbee.h"
#include "xbee_ll.h"

#include "cmsis_os.h"
#include "config.h"

uint16_t XBEE_panID;
uint8_t XBEE_chanID;
uint64_t XBEE_uid;

char tx_frame_buffer[18+20]; /* space for a tx frame with 20 bytes of data */
char rx_frame_buffer[18+0x80]; /* space for a rx frame with 0x80 bytes of data (max possible rx frame length) */

#define XBEE_TIMEOUT_FOR_STATUS_FRAME		500 // 500 ms for receiving a TX or AT status frame

int XBEE_EnterCommandMode_PARANOIA() {
	char buffer[5];
	uint8_t index;
	int status;

	for (index=0; index < 5; index++)
		buffer[index]=0;

	// flush eventual furbish
	status=XBEE_LL_ReceiveData(buffer, 1, 50); // 50ms

	/* Enter AT command mode */
	if (XBEE_LL_SendData("+++",strlen("+++")) != XBEE_LL_OK)
		return XBEE_CONFIG_ERROR;

	status=XBEE_LL_ReceiveData(buffer, 3, 2000); // 50ms
	if ((status!=XBEE_LL_ERROR_RX_TIMEOUT) && (status!=XBEE_LL_OK)) // no timeout
		return XBEE_CONFIG_ERROR;

	if ((buffer[1]!='O') && (buffer[1]!='K')) // error setting configuration mode
		return XBEE_CONFIG_ERROR;

	// wait for an eventual \r at the end
	status=XBEE_LL_ReceiveData(buffer, 1, 50); // 50ms
	if ((status!=XBEE_LL_ERROR_RX_TIMEOUT)&& (status!=XBEE_LL_OK))
		return XBEE_CONFIG_ERROR;

	return XBEE_OK;
}

int XBEE_EnterCommandMode() {
	char buffer[5];
	uint8_t index;
	int status;

	for (index=0; index < 5; index++)
		buffer[index]=0;

	/* Enter AT command mode */
	if (XBEE_LL_SendData("+++",strlen("+++")) != XBEE_LL_OK)
		return XBEE_CONFIG_ERROR;

	status=XBEE_LL_ReceiveData(buffer, 3, 2000); // 2s
	if (status!=XBEE_LL_OK) // timeout
		return XBEE_CONFIG_ERROR;

	if (strstr(buffer, "OK")==NULL) // error setting configuration mode
		return XBEE_CONFIG_ERROR;

	return XBEE_OK;
}

int XBEE_SetATCommand(char* atcmd) {
	char buffer[5];
	uint8_t index;

	for (index=0; index < 5; index++)
		buffer[index]=0;

	/* Send AT command  */
	if (XBEE_LL_SendData(atcmd,strlen(atcmd)) != XBEE_LL_OK)
		return XBEE_AT_CMD_ERROR;

	if (XBEE_LL_ReceiveData(buffer, 3, 500) != XBEE_LL_OK) // Timeout: 500 ms
		return XBEE_AT_CMD_ERROR;

	if (strcmp(buffer,"OK\r")!=0) // error setting configuration mode
		return XBEE_AT_CMD_ERROR;

	return XBEE_OK;
}

int XBEE_GetATValue(char* atcmd, char* value) {
	char tmp;
	uint8_t index;

	for (index=0; index < 20; index++)
		value[index]=0;

	/* Send AT Cmd */
	if (XBEE_LL_SendData(atcmd,strlen(atcmd)) != XBEE_LL_OK)
		return XBEE_AT_CMD_ERROR;

	/* Problem. We don't know the length of received data, only that answer end with \n
	 * So, we must loop until we have received all data
	 */
	tmp = 0;
	index=0;

	while (tmp!='\r') {
		if (XBEE_LL_ReceiveData(&tmp, 1, 500) != XBEE_LL_OK) // timeout 500 ms
			return XBEE_AT_CMD_ERROR;

		if (index >=20 ) return XBEE_AT_CMD_ERROR; // too much data received
		value[index] = (char)tmp;
		index++;
	}

	return XBEE_OK;
}

void XBEE_EncodeTransmissionFrame(char* frame, char* data, uint64_t destination, uint8_t frame_id, uint8_t pan_broadcast) {
	int i;
	int data_length = strlen(data);
	uint8_t checksum;

	frame[0] = '~';
	frame[1] = (char)((uint16_t)(14+data_length)>>8);
	frame[2] = (char)((uint16_t)(14+data_length));
	frame[3] = (char)XBEE_TX_REQUEST_TYPE;
	frame[4] = (char)frame_id;

	for (i=5; i<5+8; i++) {
		frame[i] = (char)(destination >>(64-8));
		destination = destination <<8;
	}

	frame[13] = 0xFF;
	frame[14] = 0xFE;
	frame[15] = 0;
	frame[16] = pan_broadcast ? 0x2 : 0x0;

	for (i=0; i<data_length; i++) {
		frame[17+i]= data[i];
	}

	/* calcul du checksum */
	checksum =0;
	for (i=3; i<17+data_length; i++) {
		checksum += (uint8_t)frame[i];
	}

	frame[17+data_length] = 0xFF-checksum;
	frame[17+data_length+1] = 0x0; /* End string*/
}

void XBEE_EncodeATFrame(char* frame, char* at_cmd, uint8_t* data, uint8_t data_length, uint8_t frame_id ) {
	int i;
	uint8_t checksum;

	frame[0] = '~';
	frame[1] = (char)((uint16_t)(4+data_length)>>8);
	frame[2] = (char)((uint16_t)(4+data_length));
	frame[3] = (char)XBEE_LOCAL_AT_CMD_TYPE;
	frame[4] = (char)frame_id;
	frame[5] = at_cmd[0];
	frame[6] = at_cmd[1];

	for (i=0; i<data_length; i++) {
		frame[7+i]= data[i];
	}

	/* calcul du checksum */
	checksum =0;
	for (i=3; i<7+data_length; i++) {
		checksum += (uint8_t)frame[i];
	}

	frame[7+data_length] = 0xFF-checksum;
	frame[7+data_length+1] = 0x0; /* End string*/
}

int XBEE_DecodeFrame(char* frame, XBEE_GENERIC_FRAME** decoded_frame) {
	uint8_t frame_type = (uint8_t)frame[3];
	uint16_t frame_length;
	uint8_t checksum;
	int i;

	frame_length = (((uint16_t)frame[1])<<8) + (uint16_t)frame[2];

	/* virification du checksum */
	checksum =0;
	for (i=3; i<3+frame_length+1; i++) {
		checksum += (uint8_t)frame[i];
	}

	if (checksum != 0xFF)
		return XBEE_INVALID_FRAME;

	switch (frame_type) {
	case XBEE_RX_PACKET_TYPE:
		*decoded_frame = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_RX_PACKET_FRAME)+(frame_length-12)+1); // +1 for 0 ending in data frame
		memset((void*)*decoded_frame, 0, sizeof(XBEE_RX_PACKET_FRAME)+(frame_length-12)+1);
		((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->type = frame_type;
		((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->data_length = frame_length-12;
		((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->options = frame[14];

		((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->source_addr =0;
		for (i=0; i<8; i++) {
			((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->source_addr = ((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->source_addr<<8;
			((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->source_addr +=(uint64_t)frame[4+i];
		}

		for (i=0; i<((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->data_length; i++) {
			((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->data[i] =(char)frame[15+i];
		}

		((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->data[((XBEE_RX_PACKET_FRAME*)(*decoded_frame))->data_length]=0x0; // 0 ending frame

		break;
	case XBEE_MODEM_STATUS_TYPE:
		*decoded_frame = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_MODEM_STATUS_FRAME));
		((XBEE_MODEM_STATUS_FRAME*)(*decoded_frame))->type = frame_type;
		((XBEE_MODEM_STATUS_FRAME*)(*decoded_frame))->status = frame[4];

		break;
	case XBEE_TX_STATUS_TYPE:
	case XBEE_EXTENDED_TX_STATUS_TYPE:
		*decoded_frame = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_TX_STATUS_FRAME));
		((XBEE_TX_STATUS_FRAME*)(*decoded_frame))->type = frame_type;
		((XBEE_TX_STATUS_FRAME*)(*decoded_frame))->frame_id = frame[4];
		if (frame_type == XBEE_TX_STATUS_TYPE) {
			((XBEE_TX_STATUS_FRAME*)(*decoded_frame))->status = frame[5];
			((XBEE_TX_STATUS_FRAME*)(*decoded_frame))->retry_count = 0;
		} else {
			((XBEE_TX_STATUS_FRAME*)(*decoded_frame))->status = frame[8];
			((XBEE_TX_STATUS_FRAME*)(*decoded_frame))->retry_count = frame[7];
		}

		break;
	case XBEE_AT_CMD_RESPONSE_TYPE:
		*decoded_frame = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_AT_CMD_RESPONSE_FRAME)+(frame_length-5));
		((XBEE_AT_CMD_RESPONSE_FRAME*)(*decoded_frame))->type = frame_type;
		((XBEE_AT_CMD_RESPONSE_FRAME*)(*decoded_frame))->data_length = frame_length-5;
		((XBEE_AT_CMD_RESPONSE_FRAME*)(*decoded_frame))->status = frame[7];

		for (i=0; i<((XBEE_AT_CMD_RESPONSE_FRAME*)(*decoded_frame))->data_length; i++) {
			((XBEE_AT_CMD_RESPONSE_FRAME*)(*decoded_frame))->data[i] =(uint8_t)frame[8+i];
		}
		break;
	default:
		*decoded_frame = (XBEE_GENERIC_FRAME*)malloc(sizeof(XBEE_GENERIC_FRAME));
		((XBEE_GENERIC_FRAME*)(*decoded_frame))->type = frame_type;
		return XBEE_INVALID_FRAME;
	};

	return XBEE_OK;
}

int XBEE_SendFrame (char* frame) {
	int frame_length;

	frame_length = (((int)frame[1]<<8))+(int)frame[2];
	frame_length = frame_length + 4;

	if (XBEE_LL_SendData(frame,frame_length) != XBEE_LL_OK)
		return XBEE_TX_ERROR;

	return XBEE_OK;
}

int XBEE_GetFrame (char* frame, int timeout) {
	int status;

	status = XBEE_LL_ReceiveData(frame, -1,timeout);
	if (status == XBEE_LL_ERROR_RX_TIMEOUT)
		return XBEE_RX_TIMEOUT;
	else if (status != XBEE_LL_OK)
		return XBEE_RX_ERROR;

	return XBEE_OK;
}

int XBEE_ConfigureDevice(void) {
#define RXBUFFERSIZE 30
	if (XBEE_EnterCommandMode()!=XBEE_OK)
		return XBEE_CONFIG_ERROR;

	/* Now configure the device
	 * 1 - Enable API mode (AP)
	 * 2 - Change baudrate to 57600 (BD)
	 * 3 - Get UID of Device (SH+SL)
	 * 4 - End AT command mode (CN)
	 */

	// Set API mode
	if (XBEE_SetATCommand("ATAP=1\r")!=XBEE_OK)
		return XBEE_CONFIG_ERROR;

	// disable legacy mode for API mode
	if (XBEE_SetATCommand("ATAO=0\r")!=XBEE_OK)
		return XBEE_CONFIG_ERROR;

	// Set baudrate to 115200
	if (XBEE_SetATCommand("ATBD=7\r")!=XBEE_OK)
		return XBEE_CONFIG_ERROR;

	// Finally, exit configuration mode
	if (XBEE_SetATCommand("ATCN\r")!=XBEE_OK)
		return XBEE_CONFIG_ERROR;

	return XBEE_OK;
}

int XBEE_Init (void) {
	/* First, init GPIO */
	XBEE_LL_ConfigureGPIO();

	/* then configure timer for timeout */
	//XBEE_LL_ConfigureTimer();

	/* Then, setup usart at 9600 bauds and configure the device */
	XBEE_LL_ConfigureUart(XBEE_USART, 9600);
	if (XBEE_ConfigureDevice() != XBEE_OK)
		return XBEE_CONFIG_ERROR;

	/* If it is OK, reconf USART to 115200 bauds */
	XBEE_LL_ConfigureUart(XBEE_USART, 115200);

	/* Wait 100 ms for xbee module to reconf */
	//HAL_Delay(100);
	vTaskDelay(msToTicks(100));

	/* Xbee module is ready to be used */
	return XBEE_OK;
}

int XBEE_SendData(uint64_t destination, uint8_t frame_id, uint8_t pan_broadcast, char* data, uint8_t *status) {
	XBEE_GENERIC_FRAME* tx_status;

	// Format frame for sending data
	XBEE_EncodeTransmissionFrame(tx_frame_buffer, data, destination, frame_id, pan_broadcast);
	// Send Frame
	if (XBEE_SendFrame(tx_frame_buffer) != XBEE_OK)
		return XBEE_TX_ERROR;

	// Wait for transmit status
	if (XBEE_GetFrame(rx_frame_buffer, XBEE_TIMEOUT_FOR_STATUS_FRAME) != XBEE_OK) //500ms
		return XBEE_RX_ERROR;

	// Decode frame
	if (XBEE_DecodeFrame(rx_frame_buffer, &tx_status) != XBEE_OK)
		return XBEE_INVALID_FRAME;

	if ((tx_status->type != XBEE_TX_STATUS_TYPE) && (tx_status->type != XBEE_EXTENDED_TX_STATUS_TYPE)) {
		// Free frame structure
		free((void*)tx_status);

		return XBEE_INVALID_FRAME;
	}

	*status = ((XBEE_TX_STATUS_FRAME*)tx_status)->status;

	// Free frame structure
	free((void*)tx_status);

	return XBEE_OK;
}

int XBEE_GetData (XBEE_GENERIC_FRAME** frame, int timeout) {
	int status;
	status = XBEE_LL_ReceiveData(rx_frame_buffer, -1, timeout);

	if (status == XBEE_LL_ERROR_RX_TIMEOUT)
		return XBEE_RX_TIMEOUT;
	else if (status != XBEE_LL_OK)
		return XBEE_RX_ERROR;

	if (XBEE_DecodeFrame(rx_frame_buffer, frame) != XBEE_OK) {
		free (*frame);
		return XBEE_INVALID_FRAME;
	}

	return XBEE_OK;
}

int XBEE_GetATValueU32(char* at_cmd, uint32_t *value, uint8_t *status) {
	XBEE_GENERIC_FRAME* at_status;
	int com_status;

	// Format frame for sending data
	XBEE_EncodeATFrame(tx_frame_buffer, at_cmd, (uint8_t*)0x0, 0, 1);

	// Send Frame
	if (XBEE_SendFrame(tx_frame_buffer) != XBEE_OK)
		return XBEE_TX_ERROR;

	// Wait for transmit status
	com_status = XBEE_GetFrame(rx_frame_buffer, XBEE_TIMEOUT_FOR_STATUS_FRAME);
	if (com_status != XBEE_OK)
		return com_status;

	// Decode frame
	if (XBEE_DecodeFrame(rx_frame_buffer, &at_status) != XBEE_OK)
		return XBEE_INVALID_FRAME;

	if (at_status->type != XBEE_AT_CMD_RESPONSE_TYPE) {
		// Free frame structure
		free((void*)at_status);

		return XBEE_INVALID_FRAME;
	}

	*status = ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->status;

	*value = 0;
	for (int i=0; i<((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data_length; i++) {
		*value = (*value)<<8;
		*value += ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data[i];
	}

	// Free frame structure
	free((void*)at_status);

	return XBEE_OK;
}

int XBEE_GetATValueU16(char* at_cmd, uint16_t *value, uint8_t *status) {
	XBEE_GENERIC_FRAME* at_status;
	int com_status;

	// Format frame for sending data
	XBEE_EncodeATFrame(tx_frame_buffer, at_cmd, (uint8_t*)0x0, 0, 1);

	// Send Frame
	if (XBEE_SendFrame(tx_frame_buffer) != XBEE_OK)
		return XBEE_TX_ERROR;

	// Wait for transmit status
	com_status = XBEE_GetFrame(rx_frame_buffer, XBEE_TIMEOUT_FOR_STATUS_FRAME);
	if (com_status != XBEE_OK)
		return com_status;

	// Decode frame
	if (XBEE_DecodeFrame(rx_frame_buffer, &at_status) != XBEE_OK)
		return XBEE_INVALID_FRAME;

	if (at_status->type != XBEE_AT_CMD_RESPONSE_TYPE) {
		// Free frame structure
		free((void*)at_status);

		return XBEE_INVALID_FRAME;
	}

	*status = ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->status;

	*value = 0;
	if (((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data_length ==1)
		*value = ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data[0];
	else if (((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data_length ==2) {
		*value = ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data[0];
		*value = (*value)<<8;
		*value += ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data[1];
	}

	// Free frame structure
	free((void*)at_status);

	return XBEE_OK;
}

int XBEE_GetATValueU8(char* at_cmd, uint8_t *value, uint8_t *status) {
	XBEE_GENERIC_FRAME* at_status;
	int com_status;

	// Format frame for sending data
	XBEE_EncodeATFrame(tx_frame_buffer, at_cmd, (uint8_t*)0x0, 0, 1);

	// Send Frame
	if (XBEE_SendFrame(tx_frame_buffer) != XBEE_OK)
		return XBEE_TX_ERROR;

	// Wait for transmit status
	com_status = XBEE_GetFrame(rx_frame_buffer, XBEE_TIMEOUT_FOR_STATUS_FRAME);
	if (com_status != XBEE_OK)
		return com_status;

	// Decode frame
	if (XBEE_DecodeFrame(rx_frame_buffer, &at_status) != XBEE_OK)
		return XBEE_INVALID_FRAME;

	if (at_status->type != XBEE_AT_CMD_RESPONSE_TYPE) {
		// Free frame structure
		free((void*)at_status);

		return XBEE_INVALID_FRAME;
	}

	*status = ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->status;

	*value = 0;
	if (((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data_length ==1)
		*value = ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->data[0];
	else
		return XBEE_INVALID_FRAME;

	// Free frame structure
	free((void*)at_status);

	return XBEE_OK;
}

int XBEE_SetATValueU16(char* at_cmd, uint16_t value, uint8_t *status) {
	XBEE_GENERIC_FRAME* at_status;
	uint8_t data[2];
	int com_status;

	// Format frame for sending data
	data[0] = (uint8_t)(value >>8);
	data[1] = (uint8_t)(value & 0xFF);
	XBEE_EncodeATFrame(tx_frame_buffer, at_cmd, data, 2, 1);

	// Send Frame
	if (XBEE_SendFrame(tx_frame_buffer) != XBEE_OK)
		return XBEE_TX_ERROR;

	// Wait for transmit status
	com_status = XBEE_GetFrame(rx_frame_buffer, XBEE_TIMEOUT_FOR_STATUS_FRAME);
	if (com_status != XBEE_OK)
		return com_status;

	// Decode frame
	if (XBEE_DecodeFrame(rx_frame_buffer, &at_status) != XBEE_OK)
		return XBEE_INVALID_FRAME;

	if (at_status->type != XBEE_AT_CMD_RESPONSE_TYPE) {
		// Free frame structure
		free((void*)at_status);

		return XBEE_INVALID_FRAME;
	}

	*status = ((XBEE_AT_CMD_RESPONSE_FRAME*)at_status)->status;

	// Free frame structure
	free((void*)at_status);

	return XBEE_OK;
}

int XBEE_SetChannel(uint8_t channel) {
	uint8_t status;
	int com_status;

	if ((channel < XBEE_CHANNEL_FIRST) || (channel > XBEE_CHANNEL_LAST)) // Invalid channel value
		return XBEE_AT_CMD_ERROR;

	com_status = XBEE_SetATValueU16("CH", (uint16_t)channel, &status);
	if (com_status != XBEE_OK)
		return com_status;

	if (status != XBEE_AT_STATUS_SUCCESS)
		return XBEE_AT_CMD_ERROR;

	return XBEE_OK;
}

int XBEE_SetPanID(uint16_t panid) {
	uint8_t status;
	int com_status;

	com_status=XBEE_SetATValueU16("ID", panid, &status);
	if (com_status != XBEE_OK)
		return com_status;

	if (status != XBEE_AT_STATUS_SUCCESS)
		return XBEE_AT_CMD_ERROR;

	return XBEE_OK;
}

int XBEE_GetRSSI(uint8_t *rssi) {
	uint8_t status;
	int com_status;

	com_status=XBEE_GetATValueU8("DB", rssi, &status);
	if (com_status != XBEE_OK)
		return com_status;

	if (status != XBEE_AT_STATUS_SUCCESS)
		return XBEE_AT_CMD_ERROR;

	return XBEE_OK;
}

int XBEE_GetUID(uint64_t *uid) {
	uint8_t status;
	uint32_t id=0;
	int com_status;

	com_status = XBEE_GetATValueU32("SH", &id, &status);
	if (com_status != XBEE_OK)
		return com_status;

	*uid = ((uint64_t)id<<32);

	com_status = XBEE_GetATValueU32("SL", &id, &status);
	if (com_status != XBEE_OK)
		return com_status;

	*uid += id;

	if (status != XBEE_AT_STATUS_SUCCESS)
		return XBEE_AT_CMD_ERROR;

	return XBEE_OK;
}

