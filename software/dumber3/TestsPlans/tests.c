/*
 * tests.c
 *
 *  Created on: 28 sept. 2023
 *      Author: dimercur
 */

#include "tests.h"
#include "application.h"
#include "timers.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

#include "moteurs.h"
#include "leds.h"
#include "xbee.h"
#include "batterie.h"
#include "messages.h"

StaticTask_t xTaskBasicTests;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackBasicTests[ STACK_SIZE ];
TaskHandle_t xHandleBasicTests = NULL;

typedef enum {
	LED_Tests=1,
	XBEE_Tests,
	COMMANDS_Tests,
	BATTERY_Tests,
	MOTEURS_Tests,
	MISC_Tests
} TESTS_Type;

TESTS_Type TESTS_Nbr=COMMANDS_Tests; // Number indicating which test is being run

void TESTS_BasicTests(void* params);

void TESTS_Init(void) {
	/* Init Application */

	/* Init des messages box */
	MESSAGE_Init();

	/* Init de l'afficheur */
	LEDS_Init();

	/* Init de la partie RF / reception des messages */
	XBEE_Init();
	BATTERIE_Init();
	MOTEURS_Init();

	/* Tests starts here */
	xHandleBasicTests = xTaskCreateStatic(
			TESTS_BasicTests,       /* Function that implements the task. */
			"TESTS Basic",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityTestsHandler,/* Priority at which the task is created. */
			xStackBasicTests,          /* Array to use as the task's stack. */
			&xTaskBasicTests);  /* Variable to hold the task's data structure. */

	vTaskResume(xHandleBasicTests);
}

void TESTS_BasicTests(void* params) {
	static LEDS_State ledsStates = leds_off;
	MESSAGE_Typedef msg;
	CMD_Generic* cmd;

	char* ans;
	char str[100];

	ledsStates = leds_run;
	MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledsStates); // show program is running

	switch (TESTS_Nbr) {
	case LED_Tests: //Leds tests

		while (ledsStates<=leds_state_unknown) {
			ledsStates++;

			MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledsStates);
			vTaskDelay(pdMS_TO_TICKS(TESTS_PERIODE)); // wait 10s
		}
		break;
	case XBEE_Tests: // Xbee tests

		while (1) {
			int length;

			msg = MESSAGE_ReadMailbox(APPLICATION_Mailbox); // Wait for a message from Xbee

			if (msg.id == MSG_ID_XBEE_CMD) {
				length = strlen((char*)msg.data) + strlen("Data received: ") +2;
				ans = (char*)malloc(length);
				ans[0] = 0; // empty string

				strncat(ans, "Data received: ", length);
				strncat(ans, (char *)msg.data, length);
				ans[length-2] = '\r';
				ans[length-1] = 0;

				MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (void*)ans);

				free(msg.data);
			}
		}

		break;
	case COMMANDS_Tests:

		while (1) {
			int length;

			msg = MESSAGE_ReadMailbox(APPLICATION_Mailbox); // Wait for a message from Xbee

			if (msg.id == MSG_ID_XBEE_CMD) {
				length = strlen((char*)msg.data) + strlen("Data received: ") +2;
				ans = (char*)malloc(length);
				ans[0] = 0; // empty string

				strncat(ans, "Data received: ", length);
				strncat(ans, (char *)msg.data, length);
				ans[length-2] = '\r';
				ans[length-1] = 0;

				MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (void*)ans);

				cmd = cmdDecode((char*)msg.data, strlen((char*)msg.data));
				free(msg.data);

				str[0]=0;
				switch (cmd->type) {
				case CMD_PING:
					snprintf(str, 99, "PING received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_RESET:

					snprintf(str, 99, "RESET received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_START_WITH_WATCHDOG:

					snprintf(str, 99, "START with Watchdog received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_RESET_WATCHDOG:

					snprintf(str, 99, "RESET Watchdog received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_GET_BATTERY:

					snprintf(str, 99, "GET_BATTERY received\r");
					cmdSendBatteryLevel(ANS_BAT_OK);
					break;
				case CMD_GET_VERSION:

					snprintf(str, 99, "GET_VERSION received\r");
					cmdSendVersion();
					break;
				case CMD_START_WITHOUT_WATCHDOG:

					snprintf(str, 99, "START without Watchdog received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_MOVE:

					snprintf(str, 99, "MOVE received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_TURN:

					snprintf(str, 99, "TURN received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_GET_BUSY_STATE:

					snprintf(str, 99, "GET_BUSY_STATE received\r");
					cmdSendBusyState(ANS_STATE_BUSY);
					break;
				case CMD_TEST:

					snprintf(str, 99, "TEST received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_DEBUG:

					snprintf(str, 99, "DEBUG received\r");
					cmdSendAnswer(ANS_OK);
					break;
				case CMD_POWER_OFF:

					snprintf(str, 99, "POWER_OFF received\r");
					cmdSendAnswer(ANS_OK);
					break;
				default:

					snprintf(str, 99, "Unknown command\r");
					cmdSendAnswer(ANS_UNKNOWN);
					break;
				}


				MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (void*)str);
			}
		}
		break;
	case BATTERY_Tests:

		while (1) {
			//char* str;

			msg = MESSAGE_ReadMailbox(APPLICATION_Mailbox); // Wait for a message from Xbee
			//str = (char*)malloc(100); /* allocate a buffer of 100 bytes */
			str[0]=0;

			switch (msg.id) {
			case MSG_ID_BAT_ADC_ERR:
				snprintf(str, 99, "ADC error received\r");
				break;
			case MSG_ID_BAT_CHARGE_COMPLETE:
				snprintf(str, 99, "Charge complete (plug in) [Level = %u]\r", *((uint16_t*)msg.data));
				break;
			case MSG_ID_BAT_CHARGE_ON:
				snprintf(str, 99, "Charging (plug in) [Level = %u]\r", *((uint16_t*)msg.data));
				break;
			case MSG_ID_BAT_CHARGE_OFF:
				snprintf(str, 99, "Not in charge (plug removed) [Level = %u]\r", *((uint16_t*)msg.data));
				break;
			case MSG_ID_BAT_CHARGE_ERR:
				snprintf(str, 99, "Charge error (plug in)\r");
				break;
			default:
				//free(str); // buffer alloué non utilisé
				break;
			}

			MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (void*)str);
		}
		break;
	case MOTEURS_Tests:

		break;
	case MISC_Tests: // test du bouton on/off

		while (1) {
			char* str;

			msg = MESSAGE_ReadMailbox(APPLICATION_Mailbox); // Wait for a message from button

			if (msg.id == MSG_ID_BUTTON_PRESSED) {
				str = (char*)malloc(100); /* allocate a buffer of 100 bytes */
				str[0]=0;

				snprintf(str, 99, "Bouton on/off appuyé\r");
				MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (void*)str);
			}
		}
		break;
	default:
		break;
	}
}
