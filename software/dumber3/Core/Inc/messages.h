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
extern QueueHandle_t BATTERIE_Mailbox;
extern QueueHandle_t XBEE_Mailbox;

#define MSG_ID_NO_MESSAGE		0xFFFF

#define MSG_ID_LED_ETAT			0

#define MSG_ID_BAT_NIVEAU		0
#define MSG_ID_BAT_CHARGE		1
#define MSG_ID_BAT_CHARGEUR_ON	2
#define MSG_ID_BAT_CHARGEUR_OFF	3
#define MSG_ID_BAT_CHARGEUR_ERR	4

#define MSG_ID_MOTEURS_MARCHE	0
#define MSG_ID_MOTEURS_ARRET	0

#define MSG_ID_XBEE_CMD_VALIDE		0
#define MSG_ID_XBEE_CMD_INVALIDE	1
#define MSG_ID_XBEE_REPONSE			2

void MESSAGE_Init(void);
MESSAGE_Typedef MESSAGE_ReadMailbox(QueueHandle_t mbx);
MESSAGE_Typedef MESSAGE_ReadMailboxNoDelay(QueueHandle_t mbx);
void MESSAGE_SendMailbox(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data);

#endif /* INC_MESSAGES_H_ */
