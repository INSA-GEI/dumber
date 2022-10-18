/*
 * statemachine.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "sequenceur.h"
#include "timers.h"
#include "string.h"
#include <stdlib.h>

StaticTask_t xTaskSequenceurMain;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackSequenceurMain[ STACK_SIZE ];
TaskHandle_t xHandleSequenceurMain = NULL;

StaticTimer_t xBufferTimerTimeout;
TimerHandle_t xHandleTimerTimeout = NULL;
void vTimerTimeoutCallback( TimerHandle_t xTimer );

void SEQUENCEUR_MainThread(void* params);
void SEQUENCEUR_TimeoutThread(void* params);
void SEQUENCEUR_StateMachine();
LEDS_State SEQUENCEUR_BatteryLevel(uint16_t voltage, char inCharge);
void SEQUENCEUR_PowerOff();

uint16_t SEQUENCEUR_CntTimeout;
uint16_t SEQUENCEUR_CntPowerOff;

typedef enum {
	stateStartup=0,
	stateIdle,
	stateRun,
	stateInCharge,
	stateInMouvement,
	stateWatchdogDisable,
	stateLowBatDisable
} SEQUENCEUR_State;

typedef struct {
	SEQUENCEUR_State state;
	CMD_Type cmd;
	uint16_t batteryVoltage;
	char batteryUpdate;
	char inCharge;
	int32_t distance;
	int32_t turns;
	int32_t motor_left;
	int32_t motor_right;
	char endOfMouvement;
	char powerOffRequired;
} SEQUENCEUR_Infos;

SEQUENCEUR_Infos systemInfos = {0};

void SEQUENCEUR_Init(void) {
	/* Create the task without using any dynamic memory allocation. */
	xHandleSequenceurMain = xTaskCreateStatic(
			SEQUENCEUR_MainThread,       /* Function that implements the task. */
			"SEQUENCEUR Main",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PrioritySequenceurMain,/* Priority at which the task is created. */
			xStackSequenceurMain,          /* Array to use as the task's stack. */
			&xTaskSequenceurMain);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleSequenceurMain);

	/* Create the task without using any dynamic memory allocation. */

	xHandleTimerTimeout = xTimerCreateStatic(
			"Seq Timer",
			//pdMS_TO_TICKS(100),
			pdMS_TO_TICKS(2000),
			pdTRUE,
			( void * ) 0,
			vTimerTimeoutCallback,
			&xBufferTimerTimeout);
	xTimerStart(xHandleTimerTimeout,0 );

	SEQUENCEUR_CntTimeout =0;
	SEQUENCEUR_CntPowerOff=0;
}

void SEQUENCEUR_MainThread(void* params) {
	MESSAGE_Typedef msg;

	char *cmd;
	CMD_Generic* decodedCmd;

	while (1) {
		msg = MESSAGE_ReadMailbox(SEQUENCEUR_Mailbox);

		switch (msg.id) {
		case MSG_ID_XBEE_CMD:
			cmd = (char*)msg.data;

			if (cmdIsValid(cmd)!= okANS)
				cmdSendAnswer(errANS);
			else {
				decodedCmd = cmdDecode(cmd);

				if (decodedCmd==CMD_DECODE_INVALID)
					cmdSendAnswer(errANS);
				else if (decodedCmd==CMD_DECODE_UNKNOWN)
					cmdSendAnswer(unknownANS);
				else {
					systemInfos.cmd = decodedCmd->type;
					if (decodedCmd->type == moveCMD)
						systemInfos.distance = ((CMD_Move*)decodedCmd)->distance;
					else if (decodedCmd->type == turnCMD)
						systemInfos.turns = ((CMD_Turn*)decodedCmd)->turns;
					else if (decodedCmd->type == setMotorCMD) {
						systemInfos.motor_left = ((CMD_SetMotor*)decodedCmd)->motor_left;
						systemInfos.motor_right = ((CMD_SetMotor*)decodedCmd)->motor_right;
					}

					free(decodedCmd);
				}
			}

			free(cmd);

			break;

		case MSG_ID_BAT_NIVEAU:
		case MSG_ID_BAT_CHARGE:
			systemInfos.batteryVoltage = *((uint16_t*)msg.data);
			systemInfos.batteryUpdate = 1;

			if (msg.id == MSG_ID_BAT_CHARGE)
				systemInfos.inCharge =1;
			else
				systemInfos.inCharge =0;
			break;

		case MSG_ID_BAT_CHARGEUR_ON:
		case MSG_ID_BAT_CHARGEUR_OFF:
			if (msg.id == MSG_ID_BAT_CHARGEUR_ON)
				systemInfos.inCharge =1;
			else
				systemInfos.inCharge =0;
			break;

		case MSG_ID_MOTEURS_ARRET:
			systemInfos.endOfMouvement= 1;
			break;

		case MSG_ID_BUTTON_PRESSED:
			systemInfos.powerOffRequired =1;
			break;
		}

		SEQUENCEUR_StateMachine();
	}
}

void SEQUENCEUR_StateMachine() {
	LEDS_State ledState = leds_off;

	if (systemInfos.inCharge) {
		systemInfos.state = stateInCharge;
	} else if (systemInfos.batteryUpdate) {
		ledState = SEQUENCEUR_BatteryLevel(systemInfos.batteryVoltage, systemInfos.inCharge);

		if (ledState == leds_niveau_bat_0)
			systemInfos.state= stateLowBatDisable;
	}

	switch (systemInfos.cmd) { // commands common to every state
	case pingCMD:
		break;
	case getVersionCMD:
		break;
	case getBatteryVoltageCMD:
		break;
	case powerOffCMD:
		break;
	case resetCMD:
		break;
	case busyStateCMD:
		break;
	case testCMD:
		break;
	case debugCMD:
		break;
	default: // commands no common for every states
		break;
	}

	switch (systemInfos.state) {
	case stateStartup:
		if (systemInfos.batteryUpdate) {
			ledState = SEQUENCEUR_BatteryLevel(systemInfos.batteryVoltage, systemInfos.inCharge);
			MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, SEQUENCEUR_Mailbox, (void*)&ledState);

			vTaskDelay(pdMS_TO_TICKS(2000)); // wait 2s

			systemInfos.state= stateIdle;
			ledState = leds_idle;
			MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, SEQUENCEUR_Mailbox, (void*)&ledState);
		}
		break;
	case stateIdle:
		if (systemInfos.powerOffRequired)
			SEQUENCEUR_PowerOff();
		break;
	case stateRun:
		if (systemInfos.powerOffRequired)
			SEQUENCEUR_PowerOff();
		break;
	case stateInCharge:
		if (!systemInfos.inCharge) {
			systemInfos.state = stateIdle;
			ledState = leds_idle;
			MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, SEQUENCEUR_Mailbox, (void*)&ledState);
		} else if (systemInfos.batteryUpdate) {
			ledState = SEQUENCEUR_BatteryLevel(systemInfos.batteryVoltage, systemInfos.inCharge);
			MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, SEQUENCEUR_Mailbox, (void*)&ledState);
		}
		break;
	case stateInMouvement:
		if (systemInfos.powerOffRequired)
			SEQUENCEUR_PowerOff();
		break;
	case stateWatchdogDisable:
		if (systemInfos.powerOffRequired)
			SEQUENCEUR_PowerOff();
		break;
	case stateLowBatDisable:
		ledState = leds_charge_bat_0;
		MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, SEQUENCEUR_Mailbox, (void*)&ledState);

		vTaskDelay(pdMS_TO_TICKS(2000)); // wait 2s

		SEQUENCEUR_PowerOff();
		break;
	}

	systemInfos.batteryUpdate = 0;
	systemInfos.cmd =0;
	systemInfos.endOfMouvement =0;
	systemInfos.powerOffRequired=0;
}

LEDS_State SEQUENCEUR_BatteryLevel(uint16_t voltage, char inCharge) {
	LEDS_State ledState=leds_niveau_bat_0;

	return ledState;
}

void SEQUENCEUR_PowerOff() {

}

/* This task is called every 2s */
void vTimerTimeoutCallback( TimerHandle_t xTimer ) {
}
