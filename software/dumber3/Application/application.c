/*
 * statemachine.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "application.h"
#include "timers.h"
#include "string.h"
#include <stdlib.h>

#include "moteurs.h"
#include "leds.h"
#include "xbee.h"
#include "batterie.h"
#include "messages.h"

typedef enum {
	stateStartup=0,
	stateIdle,
	stateRun,
	stateInCharge,
	stateInMouvement,
	stateWatchdogDisable,
	stateLowBatDisable
}  APPLICATION_State;

typedef struct {
	APPLICATION_State state;
	uint8_t cmd;
	uint16_t batteryVoltage;
	char batteryUpdate;
	char inCharge;
	int32_t distance;
	int32_t turns;
	int32_t motor_left;
	int32_t motor_right;
	char endOfMouvement;
	char powerOffRequired;
	uint16_t senderAddress;
	uint8_t rfProblem;

} APPLICATION_Infos;

typedef struct {
	uint32_t startupCnt;
	uint32_t inactivityCnt;
	uint32_t watchdogCnt;
	char watchdogEnabled;
	char watchdogMissedCnt;

} APPLICATION_Timeout;

StaticTask_t xTaskApplicationMain;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackApplicationMain[ STACK_SIZE ];
TaskHandle_t xHandleApplicationMain = NULL;

StaticTimer_t xBufferTimerTimeout;
TimerHandle_t xHandleTimerTimeout = NULL;
void vTimerTimeoutCallback( TimerHandle_t xTimer );

void LEDS_Tests();

void APPLICATION_MainThread(void* params);
void APPLICATION_TimeoutThread(void* params);
void APPLICATION_StateMachine();
LEDS_State APPLICATION_BatteryLevel(uint8_t voltage, APPLICATION_State state);
void APPLICATION_PowerOff();
void APPLICATION_TransitionToNewState(APPLICATION_State new_state);

APPLICATION_Infos systemInfos = {0};
APPLICATION_Timeout systemTimeout = {0};

void APPLICATION_Init(void) {
	/* Init des messages box */
	MESSAGE_Init();

	/* Init de l'afficheur */
	LEDS_Init();

	/* Init de la partie RF / reception des messages */
	XBEE_Init();
	BATTERIE_Init();
	MOTEURS_Init();

	/* Create the task without using any dynamic memory allocation. */
	xHandleApplicationMain = xTaskCreateStatic(
			APPLICATION_MainThread,       /* Function that implements the task. */
			"APPLICATION Main",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityApplicationHandler,/* Priority at which the task is created. */
			xStackApplicationMain,          /* Array to use as the task's stack. */
			&xTaskApplicationMain);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleApplicationMain);

	/* Create a periodic task without using any dynamic memory allocation. */
	xHandleTimerTimeout = xTimerCreateStatic(
			"Seq Timer",
			pdMS_TO_TICKS(APPLICATION_PERIODE),
			pdTRUE,
			( void * ) 0,
			vTimerTimeoutCallback,
			&xBufferTimerTimeout);
	xTimerStart(xHandleTimerTimeout,0 );
}

void APPLICATION_MainThread(void* params) {
	MESSAGE_Typedef msg;
	char* receivedCMD;
	CMD_Generic* decodedCmd;

	while (1) {
		msg = MESSAGE_ReadMailbox(APPLICATION_Mailbox);

		switch (msg.id) {
		case MSG_ID_XBEE_CMD:
			receivedCMD = (char*)msg.data;

			if (receivedCMD != NULL) {
				decodedCmd = cmdDecode(receivedCMD,strlen(receivedCMD));

				if (decodedCmd==CMD_DECODE_UNKNOWN)
					cmdSendAnswer(ANS_UNKNOWN);
				else {
					systemInfos.cmd = decodedCmd->type;
					systemTimeout.inactivityCnt = 0;

					/* Manage answer to command, when possible
					 * (further treatment of the command will be done in APPLICATION_StateMachine) */
					switch (decodedCmd->type) {
					case CMD_PING:
					case CMD_TEST:
					case CMD_DEBUG:
					case CMD_POWER_OFF:
						cmdSendAnswer(ANS_OK);
						break;
					case CMD_GET_BATTERY:
						cmdSendBatteryLevel(systemInfos.batteryVoltage);
						break;
					case CMD_GET_VERSION:
						cmdSendVersion();
						break;
					case CMD_GET_BUSY_STATE:
						if (systemInfos.state == stateInMouvement)
							cmdSendBusyState(0x1);
						else
							cmdSendBusyState(0x0);
						break;
					case CMD_MOVE:
						systemInfos.distance = ((CMD_Move*)decodedCmd)->distance;
						break;
					case CMD_TURN:
						systemInfos.turns = ((CMD_Turn*)decodedCmd)->turns;
						break;
					default:
						/* All others commands must be processed in state machine
						 * in order to find what action to do and what answer to give
						 */
						break;
					}
					free(decodedCmd);
				}
				break;

		default:
			break;
			}

			break;

		case MSG_ID_BAT_CHARGE_OFF:
		case MSG_ID_BAT_CHARGE_COMPLETE:
			systemInfos.batteryVoltage = *((uint16_t*)msg.data);
			systemInfos.batteryUpdate = 1;

			if (msg.id == MSG_ID_BAT_CHARGE_COMPLETE)
				systemInfos.inCharge =1;
			else
				systemInfos.inCharge =0;
			break;

		case MSG_ID_BAT_CHARGE_ON:
				systemInfos.inCharge =1;
			break;

		case MSG_ID_MOTEURS_STOP:
			systemInfos.endOfMouvement= 1;
			break;

		case MSG_ID_BUTTON_PRESSED:
			systemInfos.powerOffRequired =1;
			break;
		}

		APPLICATION_StateMachine();
	}
}

void APPLICATION_StateMachine() {
	LEDS_State ledState = leds_off;

	if (systemInfos.powerOffRequired)
		APPLICATION_PowerOff(); // system will halt here

	if ((systemInfos.inCharge) && (systemInfos.state != stateInCharge)) {
		APPLICATION_TransitionToNewState(stateInCharge);
	}

	if (systemInfos.batteryUpdate) {
		ledState = APPLICATION_BatteryLevel(systemInfos.batteryVoltage, systemInfos.state);

		if (ledState == leds_niveau_bat_0)
			APPLICATION_TransitionToNewState(stateLowBatDisable);
		else if (systemInfos.state==stateStartup) {
			MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledState);
		}
	}

	if (systemInfos.cmd != CMD_NONE) {
		/* a newer command just arrived, process it
		 * in this switch/case, we only care about state transition and command execution
		 */
		switch (systemInfos.cmd) {
		case CMD_RESET:
			if ((systemInfos.state == stateIdle) ||
					(systemInfos.state == stateRun) ||
					(systemInfos.state == stateInMouvement)) {
				/* command RESET is only applicable in those 3 states, otherwise it is rejected */
				cmdSendAnswer(ANS_OK);
				APPLICATION_TransitionToNewState(stateIdle);
			} else
				cmdSendAnswer(ANS_ERR);
			break;
		case CMD_START_WITH_WATCHDOG:
		case CMD_START_WITHOUT_WATCHDOG:
			if (systemInfos.state == stateIdle) {
				/* only state where START cmd is accepted */
				cmdSendAnswer(ANS_OK);

				APPLICATION_TransitionToNewState(stateRun);

				if (systemInfos.cmd == CMD_START_WITH_WATCHDOG) {
					systemTimeout.watchdogEnabled = 1;
					systemTimeout.watchdogCnt=0;
					systemTimeout.watchdogMissedCnt=0;
				}
			} else
				cmdSendAnswer(ANS_ERR);
			break;
		case CMD_RESET_WATCHDOG:
			if ((systemInfos.state == stateRun) || (systemInfos.state == stateInMouvement)) {
				if ((systemTimeout.watchdogEnabled ==0 ) ||
						((systemTimeout.watchdogCnt>=APPLICATION_WATCHDOG_MIN) && (systemTimeout.watchdogCnt<=APPLICATION_WATCHDOG_MAX)))
					cmdSendAnswer(ANS_OK);
				else
					cmdSendAnswer(ANS_ERR);

				systemTimeout.watchdogCnt =0;
			}
			break;
		case CMD_MOVE:
		case CMD_TURN:
			if (systemInfos.state == stateRun) { // only state where MOVE or TURN cmds are accepted

				if (((systemInfos.cmd == CMD_MOVE) && (systemInfos.distance !=0)) ||
						((systemInfos.cmd == CMD_TURN) && (systemInfos.turns !=0))) {
					systemInfos.endOfMouvement = 0;
					cmdSendAnswer(ANS_OK);
					APPLICATION_TransitionToNewState(stateInMouvement);
				} // if TURN and MOVE are sent without parameter, do nothing: we are still in run state
			} else if (systemInfos.state == stateInMouvement) { // in this state, MOVE and TURN cmds are accepted only if they come with no parameter
				if (((systemInfos.cmd == CMD_MOVE) && (systemInfos.distance ==0)) ||
						((systemInfos.cmd == CMD_TURN) && (systemInfos.turns ==0))) {
					systemInfos.endOfMouvement = 1;
					cmdSendAnswer(ANS_OK);
				}
			} else
				cmdSendAnswer(ANS_ERR);
			break;
		default: // commands no common for every states
			break;
		}
	}

	if (systemInfos.endOfMouvement) {
		APPLICATION_TransitionToNewState(stateRun);
	}

	if (systemInfos.state== stateInCharge) {
		if (!systemInfos.inCharge)
			APPLICATION_TransitionToNewState(stateIdle);
		else if (systemInfos.batteryUpdate) {
			APPLICATION_TransitionToNewState(stateInCharge);
		}
	}

	systemInfos.batteryUpdate = 0;
	systemInfos.cmd =0;
	systemInfos.endOfMouvement =0;
	systemInfos.powerOffRequired=0;
}

void APPLICATION_TransitionToNewState(APPLICATION_State new_state) {
	LEDS_State ledState = leds_off;
	uint32_t data;

	switch (new_state) {
	case stateStartup:
		// nothing to do here
		break;
	case stateIdle:
		ledState = leds_idle;
		MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledState);

		MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_STOP, APPLICATION_Mailbox, (void*)NULL);
		systemTimeout.watchdogEnabled=0;
		break;
	case stateRun:
		ledState = leds_run;
		MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledState);

		MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_STOP, APPLICATION_Mailbox, (void*)NULL);
		break;
	case stateInMouvement:
		ledState = leds_run;
		MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledState);

		if (systemInfos.cmd == CMD_MOVE) {
			data = systemInfos.distance;
			MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_MOVE, APPLICATION_Mailbox, (void*)&data);
		} else { /* obviously, cmd is CMD_TURN */
			data = systemInfos.turns;
			MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_TURN, APPLICATION_Mailbox, (void*)&data);
		}
		break;
	case stateInCharge:
		ledState = APPLICATION_BatteryLevel(systemInfos.batteryVoltage, systemInfos.inCharge);
		MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledState);
		systemTimeout.watchdogEnabled=0;
		break;
	case stateWatchdogDisable:
		ledState = leds_erreur_1;
		MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledState);
		systemTimeout.watchdogEnabled=0;
		break;
	case stateLowBatDisable:
		ledState = leds_charge_bat_0;
		MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, APPLICATION_Mailbox, (void*)&ledState);
		systemTimeout.watchdogEnabled=0;

		vTaskDelay(pdMS_TO_TICKS(4000)); // wait 4s

		/* send a message Button_Pressed to enable power off */
		MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BUTTON_PRESSED, APPLICATION_Mailbox, (void*)NULL);
		break;
	default:
		break;
	}

	systemInfos.state = new_state;
}

const uint8_t APPLICATION_NiveauBatteryNormal[5] = {
		0
};

const uint8_t APPLICATION_NiveauBatteryCharge[5] = {
		0
};

LEDS_State APPLICATION_BatteryLevel(uint8_t voltage,  APPLICATION_State state) {
	LEDS_State ledState=leds_niveau_bat_0;

	/* TODO: A faire
	 * Pour l'instant, testons les niveaux de batterie
	 */
	ledState = leds_niveau_bat_5;
	return ledState;
}

void APPLICATION_PowerOff() {
	/*
	 * TODO: a decommenter quand le code sera debuggé
	 */
	//HAL_GPIO_WritePin(SHUTDOWN_GPIO_Port, SHUTDOWN_Pin, GPIO_PIN_RESET);

	while (1){
		__WFE(); // Attente infinie que le regulateur se coupe.
	}
}

/*
 * This task is called every 100 ms
 * RQ: les constante de temps sont exprimé en ms, d'où la division par 100
 */
void vTimerTimeoutCallback( TimerHandle_t xTimer ) {
	if (systemInfos.state == stateStartup) {
		systemTimeout.startupCnt++;
		if (systemTimeout.startupCnt++>=(APPLICATION_STARTUP_DELAY/100))
			APPLICATION_TransitionToNewState(stateIdle);
	}

	systemTimeout.inactivityCnt++;
	if (systemTimeout.inactivityCnt>=(APPLICATION_INACTIVITY_TIMEOUT/100))
		/* send a message Button_Pressed to enable power off */
		MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BUTTON_PRESSED, APPLICATION_Mailbox, (void*)NULL);

	if (systemTimeout.watchdogEnabled) {
		systemTimeout.watchdogCnt++;

		if (systemTimeout.watchdogCnt>(APPLICATION_WATCHDOG_MAX/100)) {
			systemTimeout.watchdogCnt=0;
			systemTimeout.watchdogMissedCnt++;

			if (systemTimeout.watchdogMissedCnt>=(APPLICATION_WATCHDOG_MISSED_MAX/100))
				APPLICATION_TransitionToNewState(stateWatchdogDisable);
		}
	}
}
