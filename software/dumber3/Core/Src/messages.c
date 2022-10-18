/*
 * messages.c
 *
 *  Created on: 14 sept. 2022
 *      Author: dimercur
 */

#include "messages.h"
#include "stdlib.h"

#define QUEUE_SIZE 5

QueueHandle_t LEDS_Mailbox;
QueueHandle_t MOTEURS_Mailbox;
QueueHandle_t SEQUENCEUR_Mailbox;
QueueHandle_t BATTERIE_Mailbox;
QueueHandle_t XBEE_Mailbox;

void  MESSAGE_Init(void) {
	LEDS_Mailbox = xQueueCreate(QUEUE_SIZE, sizeof(MESSAGE_Typedef));
	MOTEURS_Mailbox = xQueueCreate(QUEUE_SIZE, sizeof(MESSAGE_Typedef));
	SEQUENCEUR_Mailbox = xQueueCreate(QUEUE_SIZE, sizeof(MESSAGE_Typedef));
}

MESSAGE_Typedef MESSAGE_ReadMailbox(QueueHandle_t mbx) {
	MESSAGE_Typedef msg= {0};
	char msg_received =0;

	while (!msg_received) {
		if (xQueueReceive(mbx, &msg, portMAX_DELAY)) { // un message à été reçu
			msg_received=1;

			return msg;
		}
	}

	return msg;
}

MESSAGE_Typedef MESSAGE_ReadMailboxNoDelay(QueueHandle_t mbx) {
	MESSAGE_Typedef msg= {0};

	if (!xQueueReceive(mbx, &msg, 0))  // Pas de message dans la mailbox
		msg.id=MSG_ID_NO_MESSAGE;

	return msg;
}

void MESSAGE_SendMailbox(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data){
	MESSAGE_Typedef msg;

	msg.id=id;
	msg.sender = &mbx_sender;
	msg.data=data;

	if (!xQueueSend(mbx_dest, &msg, 0)) { // envoi sans attendre
		//printf("Failed to send data, Queue full.\r\n");
	}
}

void MESSAGE_SendMailboxFromISR(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data, BaseType_t *xHigherPriorityTaskWoken) {
	MESSAGE_Typedef msg;

	msg.id=id;
	msg.sender = &mbx_sender;
	msg.data=data;

	if (!xQueueSendFromISR( mbx_dest, &msg, xHigherPriorityTaskWoken)) { // envoi sans attendre
		//printf("Failed to send data, Queue full.\r\n");
	}
}



