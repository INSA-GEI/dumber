#include "Xbee.h"
#include "common.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>

char xbee_compute_checksum(char *buffer, int length);
static void xbee_debug_send(int address, char *str);

int xbeeSend(int addr_destination, char *str)
{
		int i;
		char buffer[TRAME_SIZE_SEND];
		#ifdef __DEBUG__
		//xbee_debug_send(addr_destination, str);
		#endif /* __DEBUG__ */
		
		/* Allocation d'un buffer suffisant */
		
		if ((addr_destination>0xFFFF) || (addr_destination<0)) return XBEE_INVALID_ADDRESS;
		/* Remplissage du buffer */
		buffer[0] = 0x7E;
		buffer[1] = 0;
		buffer[2] = 7;
		buffer[3] = 0x01;
		buffer[4] = 0; // frame id set to 0 to disable ack
	
		buffer[5] = addr_destination>>8;
		buffer[6] = addr_destination&0xFF;

		buffer[7] = 0x0;
		
		for (i=0; i<3; i++){
			if((str[i]== 0x7E) ||(str[i]== 0x11) ||(str[i]== 0x13)){
				buffer[8+i] = str[i] + 1;
			}
			else if(str[i] == 0x7D){
				buffer[8+i]=str[i] + 2;
			}
			else {
				buffer[8+i] = str[i];
			}
		}
		buffer[TRAME_SIZE_SEND - 1]=xbee_compute_checksum(buffer, TRAME_SIZE_SEND-1);
		
		//HAL_UART_Transmit_DMA(&huart1,(uint8_t *)buffer, TRAME_SIZE);
		
		HAL_UART_Transmit(&huart1,(uint8_t *)buffer, TRAME_SIZE_SEND,1000);
	 return XBEE_SUCCESS;
}


char xbee_compute_checksum(char *buffer, int length){
	int i;
	unsigned char checksum=0;
	for (i =3; i<length; i++){
		checksum =checksum + (unsigned char)buffer[i];
	}
	
	return (0xFF-checksum);
}

#ifdef __DEBUG__
//static void xbee_debug_receive(void)
//{
//char *string_out;
//int i;
//ST_XBEE_FRAME frame; 
//	
//	string_out = malloc(200);
//	
//	if (string_out!=0)
//	{
//		if (xbee_unpack_frame(&frame)==XBEE_SUCCESS)
//		{
//			if (frame.apiCmd == 0x89) // TX ACK
//			{
//				if (frame.status == 0)
//					sprintf(string_out, "<= Xbee Transmit Status (0x89)\n\r\tStatus = Success\n\r");
//				else
//					sprintf(string_out, "<= Xbee Transmit Status (0x89)\n\r\tStatus = Fail (No Ack)\n\r"); 
//			}
//			else
//				sprintf (string_out, "<= Xbee Receive Packet (0x81)\n\r\tAddress = 0x%04X\n\r\tRssi = 0x%02X\n\r\tLength = %i\n\r\tData = %s\n\r", frame.address, frame.rssi, frame.data_length, frame.data);
//		}
//		
//		for (i=0; i<strlen(string_out); i++)
//		{
//			ITM_SendChar(string_out[i]);
//		}
//	
//		free (string_out);
//	}
//}

//void wait_untilData(ST_XBEE_FRAME * FRAME){
//	char TRAME[20];
//	char TRAME2[20];
//	uint8_t size;
//	while(get_whole_trame(TRAME,&size) != 0);
//	xbee_escape_chars_receipt(TRAME,TRAME2,size);
//	dmaRestart();
//	xbee_unpack_frame(TRAME2,FRAME);	
//}

#endif /* __DEBUG__ */
