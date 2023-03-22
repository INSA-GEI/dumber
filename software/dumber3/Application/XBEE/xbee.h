/*
 * xbee.h
 *
 *  Created on: 4 avr. 2022
 *      Author: dimercur
 */

#ifndef BSP_XBEE_XBEE_H_
#define BSP_XBEE_XBEE_H_

#include "stm32l4xx_hal.h"

#define XBEE_OK   					0
#define XBEE_CONFIG_ERROR   		2
#define XBEE_AT_CMD_ERROR			3
#define XBEE_RX_TIMEOUT				4
#define XBEE_RX_ERROR				5
#define XBEE_TX_ERROR				6
#define XBEE_INVALID_FRAME			7

#define XBEE_RX_PACKET_TYPE			0x90
#define XBEE_RX_EXPLICIT_TYPE		0x91
#define XBEE_TX_STATUS_TYPE			0x89
#define XBEE_AT_CMD_RESPONSE_TYPE	0x88
#define XBEE_MODEM_STATUS_TYPE		0x8A
#define XBEE_EXTENDED_TX_STATUS_TYPE	0x8B
#define XBEE_LOCAL_AT_CMD_TYPE		0x08
#define XBEE_TX_REQUEST_TYPE		0x10
#define XBEE_TX_EXPLICIT_TYPE		0x11

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

#define XBEE_CHANNEL_FIRST					0x0B
#define XBEE_CHANNEL_LAST					0x1A

#define XBEE_BROADCAST_ADDRESS				0x000000000000FFFF

#define XBEE_PANID_BROADCAST				SET
#define XBEE_NO_PANID_BROADCAST				RESET

typedef struct {
	uint8_t type;
} XBEE_GENERIC_FRAME;

typedef struct {
	uint8_t type;
	uint64_t source_addr;
	uint8_t options;
	uint8_t data_length;
	char data[];
} XBEE_RX_PACKET_FRAME;

typedef struct {
	uint8_t type;
	uint8_t frame_id;
	uint16_t at_cmd;
	uint8_t status;
	uint8_t data_length;
	char data[];
} XBEE_AT_CMD_RESPONSE_FRAME;

typedef struct {
	uint8_t type;
	uint8_t frame_id;
	uint8_t status;
	uint8_t retry_count;
} XBEE_TX_STATUS_FRAME;

typedef struct {
	uint8_t type;
	uint8_t status;
} XBEE_MODEM_STATUS_FRAME;

int XBEE_ConfigureDevice(void);
int XBEE_Init (void);

int XBEE_DecodeFrame(char* frame, XBEE_GENERIC_FRAME** decoded_frame);
int XBEE_GetData (XBEE_GENERIC_FRAME** frame, int timeout); // timeout in ms
int XBEE_SendData(uint64_t destination, uint8_t frame_id, uint8_t pan_broadcast, char* data, uint8_t *status);

int XBEE_SetChannel(uint8_t channel);
int XBEE_SetPanID(uint16_t panid);
int XBEE_GetRSSI(uint8_t *rssi);
int XBEE_GetUID(uint64_t *uid);

#endif /* BSP_XBEE_XBEE_H_ */
