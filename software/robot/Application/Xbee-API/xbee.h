/*
 * xbee.h
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#ifndef INC_XBEE_H_
#define INC_XBEE_H_

#include "application.h"

typedef enum {
	XBEE_OK=0,
	XBEE_CONFIG_ERROR=-1,
	XBEE_TX_ACK_ERROR=-2,
	XBEE_RX_TIMEOUT=-3,
	XBEE_RX_ERROR=-4,
	XBEE_TX_ERROR=-5,
	XBEE_INVALID_FRAME=-6,
	XBEE_TX_TIMEOUT=-7,
} XBEE_Status;

#define XBEE_RX_PACKET_TYPE					0x90
#define XBEE_RX_EXPLICIT_TYPE				0x91
#define XBEE_TX_STATUS_TYPE					0x89
#define XBEE_AT_CMD_RESPONSE_TYPE			0x88
#define XBEE_MODEM_STATUS_TYPE				0x8A
#define XBEE_EXTENDED_TX_STATUS_TYPE		0x8B
#define XBEE_LOCAL_AT_CMD_TYPE				0x08
#define XBEE_TX_REQUEST_TYPE				0x10
#define XBEE_TX_EXPLICIT_TYPE				0x11
#define XBEE_TX_16BIT_REQUEST_TYPE			0x01
#define XBEE_RX_16BIT_PACKET_TYPE			0x81

#define XBEE_AT_STATUS_SUCCESS				0
#define XBEE_AT_STATUS_ERROR				1
#define XBEE_AT_STATUS_INVALID_COMMAND		2
#define XBEE_AT_STATUS_INVALID_PARAMETER	3

#define XBEE_TX_STATUS_SUCCESS				0x00
#define XBEE_TX_STATUS_NO_ACK				0x01
#define XBEE_TX_STATUS_CCA_FAILURE			0x02
#define XBEE_TX_STATUS_NETWORK_NO_ACK		0x21

#define XBEE_MODEM_STATUS_HW_RST			0x00
#define XBEE_MODEM_STATUS_JOINED			0x02

#define XBEE_RX_OPTIONS_ACKNOWLEDGED		0x01
#define XBEE_RX_OPTIONS_BRODCASTED			0x02
#define XBEE_RX_OPTIONS_PAN_BROADCASTED		0x04

#define XBEE_FRAME_SOF_CHAR					0x7E // '~'

typedef struct {
	uint8_t  	type;
	uint16_t 	source_addr;
	uint8_t 	length;
	char  		ack;
	char		modem_status;
	char 		data[];
} XBEE_INCOMING_FRAME;

void XBEE_Init(void);
int XBEE_SendData(uint16_t address, char* data, int length);

#endif /* INC_XBEE_H_ */
