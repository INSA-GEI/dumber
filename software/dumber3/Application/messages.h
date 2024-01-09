/*
 * messages.h
 *
 *  Created on: 14 sept. 2022
 *      Author: dimercur
 */

#ifndef INC_MESSAGES_H_
#define INC_MESSAGES_H_

#include "application.h"
#include "queue.h"

typedef struct {
	uint16_t id;
	QueueHandle_t* sender;
	void *data;
} MESSAGE_Typedef;

extern QueueHandle_t LEDS_Mailbox;
extern QueueHandle_t MOTORS_Mailbox;
extern QueueHandle_t APPLICATION_Mailbox;
extern QueueHandle_t XBEE_Mailbox;

#define MSG_ID_NO_MESSAGE			0x00

#define MSG_ID_LED_ETAT				0x10

#define MSG_ID_BAT_CHARGE_ERR		0x20
#define MSG_ID_BAT_ADC_ERR			0x21
#define MSG_ID_BAT_LEVEL			0x22
#define MSG_ID_BAT_CRITICAL_LOW		0x23
#define MSG_ID_BAT_LOW				0x24
#define MSG_ID_BAT_MED				0x25
#define MSG_ID_BAT_HIGH				0x26
#define MSG_ID_BAT_CHARGE_LOW		0x27
#define MSG_ID_BAT_CHARGE_MED		0x28
#define MSG_ID_BAT_CHARGE_HIGH		0x29
#define MSG_ID_BAT_CHARGE_COMPLETE	0x2A
//#define MSG_ID_BAT_CHARGE_ON		0x23
//#define MSG_ID_BAT_CHARGE_OFF		0x24


#define MSG_ID_BUTTON_PRESSED		0x30

#define MSG_ID_MOTORS_STOP			0x40
#define MSG_ID_MOTORS_MOVE			0x41
#define MSG_ID_MOTORS_TURN			0x42
#define MSG_ID_MOTORS_END_OF_MOUVMENT	0x43

#define MSG_ID_XBEE_CMD				0x50
#define MSG_ID_XBEE_ANS				0x51

void MESSAGE_Init(void);
MESSAGE_Typedef MESSAGE_ReadMailbox(QueueHandle_t mbx);
MESSAGE_Typedef MESSAGE_ReadMailboxNoDelay(QueueHandle_t mbx);
void MESSAGE_SendMailbox(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data);
void MESSAGE_SendMailboxFromISR(QueueHandle_t mbx_dest, uint16_t id, QueueHandle_t mbx_sender, void *data, BaseType_t *xHigherPriorityTaskWoken);

#endif /* INC_MESSAGES_H_ */
