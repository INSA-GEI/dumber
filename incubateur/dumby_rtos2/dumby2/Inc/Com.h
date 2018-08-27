#ifndef COM_H
#define COM_H

#include "stm32f1xx_hal.h"

/*
 * TRAME 802.15.4 			<0x7E> <data lenght(2bytes)> <Frame Data> <checksum>
 */

typedef struct{
	int address;
	int apiCmd; // Emision
	int rssi; // Reception
	char data[100];
	int data_length;
	int options;
	int status; // Emission
} ST_XBEE_FRAME;



void INIT_RECEIVE(void);
short get_whole_trame(char* BufferRx, uint8_t *sizeTrame);
void dmaRestart(void);
int xbee_escape_chars_transmit(char* buffer_out, char* buffer_in, int length_in);
int xbee_escape_chars_receipt(char* buffer_in, char* buffer_out, int length_in);
int xbee_unpack_frame(char * xbee_frame_buffer,ST_XBEE_FRAME *frame);
int xbee_send_data(int addr_destination, char *str);
#endif
