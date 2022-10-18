/*
 * messages.h
 *
 *  Created on: 14 sept. 2022
 *      Author: dimercur
 */

#ifndef INC_MESSAGES_H_
#define INC_MESSAGES_H_

#include "main.h"
#include "queue.h"

typedef struct {
	uint16_t id;
	QueueHandle_t* sender;
	void *data;
} MESSAGE_Typedef;

extern QueueHandle_t LEDS_Mailbox;
extern QueueHandle_t MOTEURS_Mailbox;
extern QueueHandle_t SEQUENCEUR_Mailbox;

#define MSG_ID_NO_MESSAGE			0x00

#define MSG_ID_LED_ETAT				0x10

#define MSG_ID_BAT_NIVEAU			0x20
#define MSG_ID_BAT_CHARGE			0x21
#define MSG_ID_BAT_CHARGEUR_ON		0x22
#define MSG_ID_BAT_CHARGEUR_OFF		0x23
#define MSG_ID_BAT_CHARGEUR_ERR		0x24
#define MSG_ID_BAT_ADC_ERR			0x25

#define MSG_ID_BUTTON_PRESSED		0x30

#define MSG_ID_MOTEURS_MARCHE		0x40
#define MSG_ID_MOTEURS_ARRET		0x41

#define MSG_ID_XBEE_CMD				0x50

void MESSAGE_Init(void);
MESSAGE_Typedef MESSAGE_ReadMailbox(QueueHandle_t mbx);
MESSAGE_Typedef MESSAGE_ReadMailboxNoDelay(QueueHandle_t mbx);
void MESSAGE_SendMailbox(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data);
void MESSAGE_SendMailboxFromISR(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data, BaseType_t *xHigherPriorityTaskWoken);

#endif /* INC_MESSAGES_H_ */
