//#include <string.h>
//#include <stdlib.h>
//#include "common.h"
//#include "Com.h"
//#include "usart.h"
///*Manage Trame Reception*/



///*Local Variable*/
//uint8_t Rx[XBEE_TRAME_MAX_SIZE];
//int xbee_frame_counter=1;


///*Local prototype*/
//void wait_untilData(ST_XBEE_FRAME * FRAME);

///*Function definition*/

//short get_whole_trame(char* BufferRx, uint8_t *sizeTrame){
//	// Init
//	uint8_t dmaCnt = XBEE_TRAME_MAX_SIZE -__HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
//	uint8_t i,j,k;
//	uint8_t indexTrame=0;
//	union xbee_length{
//		uint16_t u16;
//		uint8_t u8[2];
//	};
//	union xbee_length length;
//	//Copy last trame update ?
//	
//	//Traitement
//	
//	// SEARCH FOR TRAME START DELIMITER
//	for(i=0; Rx[i] != XBEE_START_DELIMITER;i++){
//		if(dmaCnt <i)
//			return XBEE_RECEIVE_INCOMPLETE;
//	}
//	BufferRx[indexTrame] = Rx[i];
//	indexTrame++;
//	
//	// CHECK THE 2 FOLLOWING CHAR
//	for(j=i+1; j < i+3 ; j++){
//		if(dmaCnt <j)
//			return XBEE_RECEIVE_INCOMPLETE;
//		else{
//			length.u8[(i+2)-j] = Rx[j]; //length.u8[j-i-1] = Rx[j];
//			BufferRx[indexTrame]=Rx[j];
//			indexTrame++;
//		}
//			
//	}
//	// CHECK IF THE TOTALITY OF THE TRAME HAS BEEN RECEIPT
//	for(k=j; k<length.u16+5; k++){
//		if(dmaCnt <k)
//			return XBEE_RECEIVE_INCOMPLETE;
//		else{
//			BufferRx[indexTrame]=Rx[k];
//			indexTrame++;
//		}
//	}
//	*sizeTrame = indexTrame;
//	return 0;
//}

//void INIT_RECEIVE(void){
//	
//	HAL_UART_Receive_DMA(&huart1,Rx,XBEE_TRAME_MAX_SIZE);
//}
//void dmaRestart(void){
//	HAL_UART_DMAStop(&huart1);
//	HAL_UART_Receive_DMA(&huart1,Rx,XBEE_TRAME_MAX_SIZE);
//}


///*Author : S.D
// * Function that parse data in order to add escapement char.
// * Usefull for transmit only
// * */

//#ifdef __DEBUG__
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

//static void xbee_debug_send(int address, char *str)
//{
//	char *string_out;
//	int i;
//	
//	string_out = malloc (strlen(str) + strlen ("=> Xbee Transmit Request (0x01)\n\r\tAddress = 0x\n\r\tLength = \n\r\tData = \n\r")+20);
//	
//	if (string_out != 0)
//	{
//		sprintf (string_out, "=> Xbee Transmit Request (0x01)\n\r\tAddress = 0x%04X\n\r\tLength = %i\n\r\tData = %s\n\r",address, strlen(str), str);
//	
//		for (i=0; i<strlen(string_out); i++)
//		{
//			ITM_SendChar(string_out[i]);
//		}
//	
//		free (string_out);
//	}
//}
//#endif /* __DEBUG__ */

//int xbee_escape_chars_transmit(char* buffer_out, char* buffer_in, int length_in){
//	int new_length =length_in;
//	int i, i_out;
//	int length_out;
//	
//	/* Premiere passe: calcul de la longueur du nouveau buffer */
//	for (i = 1; i< length_in-1; i++)
//	{
//		if ((buffer_in[i]== 0x7E) ||
//			  (buffer_in[i]== 0x7D) ||
//		    (buffer_in[i]== 0x11) ||
//		    (buffer_in[i]== 0x13)) new_length++;
//	}
//	
//	if (length_in == new_length) /* Pas de caractere à echapper */{
//		length_out = length_in;
//	}
//	else /* il y a des caracteres à echapper */{
//		length_out = new_length;
//	}
//	buffer_out[0] = 0x7E;
//	i_out=1;
//	for (i = 1; i< length_in; i++){
//		buffer_out[i_out] = buffer_in[i];
//		
//		if ((buffer_out[i_out]== 0x7E) ||
//				(buffer_out[i_out]== 0x7D) ||
//				(buffer_out[i_out]== 0x11) ||
//				(buffer_out[i_out]== 0x13))
//		{
//			buffer_out[i_out+1]=buffer_out[i_out]^0x20;
//			buffer_out[i_out]=0x7D;
//			i_out++;
//		}
//		i_out++;
//	}
//	return length_out;
//}

//int xbee_escape_chars_receipt(char* buffer_in, char* buffer_out, int length_in){
//	int new_length =length_in;
//	int i, i_in;
//	int length_out;
//	
//	/* Premiere passe: calcul de la longueur du nouveau buffer */
//	for (i = 1; i< length_in-1; i++)
//	{
//		if ((buffer_in[i]== 0x7E) ||
//			  (buffer_in[i]== 0x7D) ||
//		    (buffer_in[i]== 0x11) ||
//		    (buffer_in[i]== 0x13)) new_length--;
//	}
//	
//	if (length_in == new_length) /* Pas de caractere à echapper */{
//		length_out = length_in;
//	}
//	else /* il y a des caracteres à echapper */{
//		length_out = new_length;
//	}
//	buffer_out[0] = 0x7E;
//	i_in=1;
//	for (i = 1; i< length_out; i++){
//		
//		if ((buffer_in[i_in]== 0x7E) ||
//				(buffer_in[i_in]== 0x7D) ||
//				(buffer_in[i_in]== 0x11) ||
//				(buffer_in[i_in]== 0x13))
//		{
//			i_in++;
//			buffer_out[i] = buffer_in[i_in]^0x20;
//		}
//		else{
//			buffer_out[i] = buffer_in[i_in];
//		}
//		i_in++;
//	}
//	return length_out;
//}


//char xbee_compute_checksum(char *buffer, int length){
//	int i;
//	unsigned char checksum=0;
//	for (i =3; i<length; i++){
//		checksum =checksum + (unsigned char)buffer[i];
//	}
//	
//	return (0xFF-checksum);
//}


//int xbee_unpack_frame(char * xbee_frame_buffer,ST_XBEE_FRAME *frame){
//	int status = XBEE_SUCCESS;
//	int length;
//	int checksum;
//	int i;
//	
//	/* Recupere la longueur de la trame (tel qu'ecrite dans la trame)*/
//	length = (int)((int)(xbee_frame_buffer[1])<<8) + (int)xbee_frame_buffer[2];
//	
//	/* Calcul le checksum */
//	checksum = xbee_compute_checksum(xbee_frame_buffer, length+1+3);
//	if (checksum != 0x00 ) 
//	{
//		return XBEE_INVALID_FRAME;
//	} 
//	
//	frame->apiCmd = xbee_frame_buffer[3];
//	
//	if (frame->apiCmd ==0x89) /* transmit frame status */
//	{
//		frame->status = xbee_frame_buffer[5];
//		
//		if ((frame->status<0) && (frame->status>4)) return XBEE_INVALID_FRAME;
//	}
//	else if (frame->apiCmd ==0x81) /* 16-bit address receive frame */
//	{
//		frame->address = (int)((int)(xbee_frame_buffer[4])<<8) + (int)xbee_frame_buffer[5];
//		
//		frame->rssi = (int)xbee_frame_buffer[6];
//		frame->options = (int)xbee_frame_buffer[7];
//		
//		if ((frame->options<0) && (frame->options>2)) return XBEE_INVALID_FRAME;
//		
//		/* Recopie la pdu */
//		for (i=0; i<(length-5); i++)
//		{
//			frame->data[i] = xbee_frame_buffer[8+i];
//		}
//		
//		frame->data[i] = 0x0;
//		frame->data_length=i;
//	}
//	else status = XBEE_INVALID_FRAME;
//	
//	return status;
//}


//int xbee_send_data(int addr_destination, char *str)
//{
//int status = XBEE_SUCCESS;

//	int i;
//	int buffer_length;

//ST_XBEE_FRAME frame;
//	ST_XBEE_FRAME FRAME;
//	#ifdef __DEBUG__
//	xbee_debug_send(addr_destination, str);
//	#endif /* __DEBUG__ */
//	
//	/* Allocation d'un buffer suffisant */
//	char buffer[strlen(str)+9];
//	char * buffer_esc;
//	
//	if (*buffer == 0x0) return XBEE_SEND_FAILED;
//	if ((addr_destination>0xFFFF) || (addr_destination<0)) return XBEE_INVALID_ADDRESS;
//	
//	/* Remplissage du buffer */
//	buffer[0] = 0x7E;
//	buffer[1] = 0;
//	buffer[2] = strlen(str)+5;
//	buffer[3] = 0x01;
//	buffer[4] = xbee_frame_counter;
//	
//	xbee_frame_counter++;
//	if (xbee_frame_counter>255) xbee_frame_counter=1;

//	buffer[5] = addr_destination>>8;
//	buffer[6] = addr_destination&0xFF;

//	buffer[7] = 0x0;
//	
//	for (i=0; i<strlen(str); i++)
//	{
//		buffer[8+i]=str[i];
//	}
//	
//	buffer[strlen(str)+9-1]=xbee_compute_checksum(buffer, strlen(str)+9-1);
//	
//	/* Allocation d'un buffer pour la reception de la chaine echappÃ©e */
//	
//	buffer_length= xbee_escape_chars_transmit(buffer_esc, buffer, strlen(str)+9);
//	
//	if (buffer_length == 0) return XBEE_SEND_FAILED;	

//	HAL_UART_Transmit_DMA(&huart1,(uint8_t *)buffer_esc, buffer_length);
//	
//	/* Attente de la reponse */
// //wait_untilData(&FRAME);
//		/* Verifions que ce soit soit ACk, soit NACK */
//	//if (FRAME.status != 0) /* -> No ACK */
//		//return -1;
//	
//	free (buffer_esc); /* On libere le buffer avant de partir */

//	
//	return status;
//	return 0;
//}

