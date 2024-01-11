/**
 ******************************************************************************
 * @file messages.c
 * @brief messages handler body
 * @author S. DI MERCURIO (dimercur@insa-toulouse.fr)
 * @date December 2023
 *
 ******************************************************************************
 * @copyright Copyright 2023 INSA-GEI, Toulouse, France. All rights reserved.
 * @copyright This project is released under the Lesser GNU Public License (LGPL-3.0-only).
 *
 * @copyright This file is part of "Dumber" project
 *
 * @copyright This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * @copyright This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * @copyright You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ******************************************************************************
 */

#include "messages.h"
#include "stdlib.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup MESSAGES
 * Messages module handles mailbox and messages communication support between tasks
 * @{
 */

/** @addtogroup MESSAGES_Private Private
 * @{
 */

#define QUEUE_SIZE 5

QueueHandle_t LEDS_Mailbox;
QueueHandle_t MOTORS_Mailbox;
QueueHandle_t APPLICATION_Mailbox;
QueueHandle_t XBEE_Mailbox;

/**
 * @brief  Function for initializing messaging system
 *
 * @param  None
 * @return None
 */
void  MESSAGE_Init(void) {
	LEDS_Mailbox = xQueueCreate(QUEUE_SIZE, sizeof(MESSAGE_Typedef));
	MOTORS_Mailbox = xQueueCreate(QUEUE_SIZE, sizeof(MESSAGE_Typedef));
	APPLICATION_Mailbox = xQueueCreate(QUEUE_SIZE, sizeof(MESSAGE_Typedef));
	XBEE_Mailbox = xQueueCreate(QUEUE_SIZE, sizeof(MESSAGE_Typedef));
}

/**
 * @brief  Return first message a given mailbox
 *
 * @param[in] mbx Mailbox reference
 * @return First message in mailbox
 *
 * @remark This function is blocking until a message is received.
 * If mailbox is not empty when calling the function, return immediatly with first message in mailbox
 */
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

/**
 * @brief  Return first message a given mailbox (non blocking)
 *
 * @param[in] mbx Mailbox reference
 * @return First message in mailbox
 *
 * @remark This function is non blocking.
 * If mailbox is empty when calling the function, a message structure is still returned with \ref MSG_ID_NO_MESSAGE
 * in message id field
 */
MESSAGE_Typedef MESSAGE_ReadMailboxNoDelay(QueueHandle_t mbx) {
	MESSAGE_Typedef msg= {0};

	if (!xQueueReceive(mbx, &msg, 0))  // Pas de message dans la mailbox
		msg.id=MSG_ID_NO_MESSAGE;

	return msg;
}

/**
 * @brief  Post a message in a mailbox with given id and data
 *
 * @param[in] mbx_dest Destination mailbox
 * @param[in] id id of message (as found in \ref msg_id_def "Message ID definitions")
 * @param[in] mbx_sender Sender mailbox (may be null if no answer is expected)
 * @param[in] data Reference to data to store in message structure
 * @return None
 *
 * @remark This function is non blocking.
 * If mailbox is full when calling the function, error will be ignored silently
 */
void MESSAGE_SendMailbox(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data){
	MESSAGE_Typedef msg;

	msg.id=id;
	msg.sender = &mbx_sender;
	msg.data=data;

	if (!xQueueSend(mbx_dest, &msg, 0)) { // envoi sans attendre
		//printf("Failed to send data, Queue full.\r\n");
	}
}

/**
 * @brief  Post, from an ISR, a message in a mailbox with given id and data
 *
 * @param[in] mbx_dest Destination mailbox
 * @param[in] id id of message (as found in \ref msg_id_def "Message ID definitions")
 * @param[in] mbx_sender Sender mailbox (may be null if no answer is expected)
 * @param[in] data Reference to data to store in message structure
 * @param[in] xHigherPriorityTaskWoken Reference to flag indicating if a context switch is to be done at end of ISR
 * @return None
 *
 * @remark This function is non blocking.
 * If mailbox is full when calling the function, error will be ignored silently
 */
void MESSAGE_SendMailboxFromISR(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data, BaseType_t *xHigherPriorityTaskWoken) {
	MESSAGE_Typedef msg;

	msg.id=id;
	msg.sender = &mbx_sender;
	msg.data=data;

	if (!xQueueSendFromISR( mbx_dest, &msg, xHigherPriorityTaskWoken)) { // envoi sans attendre
		//printf("Failed to send data, Queue full.\r\n");
	}
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
