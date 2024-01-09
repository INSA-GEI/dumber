/**
 ******************************************************************************
 * @file application.c
 * @brief application body
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

/**
 * @mainpage  Dumber 3
 *
 * Dumber is a robot used at INSA-GEI, Toulouse, France for realtime computer science teaching.
 * Robot is basically controlled by a supervisor program and move depending on commands send by supervisor.
 * Movements are controlled by a camera.
 *
 * @copyright Copyright 2023 INSA-GEI, Toulouse, France. All rights reserved.
 * @copyright This project is released under the Lesser GNU Public License (LGPL-3.0-only).
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
 */

#include "application.h"
#include "timers.h"
#include "string.h"
#include <stdlib.h>

#include "leds.h"
#include "xbee.h"
#include "messages.h"
#include "motors.h"
#include "battery.h"

#include "panic.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup APPLICATION
  * @{
  */

/** @addtogroup APPLICATION_Private Private
  * @{
  */

/** Enumeration class used by application state machine for defining current application state */
typedef enum {
	stateStartup=0,				/**< Startup state, after system power on */
	stateIdle,					/**< Idle state, after system has initialized all peripherals and is ready to handle commands */
	stateRun,					/**< Run state, after system has received and accepted "StartWithWatchdog" or "StartWithoutWatchdog" command  */
	stateInCharge,				/**< In Charge state, when a charger is plugged */
	stateInMouvement,			/**< In Movement state, when the robot is moving */
	stateWatchdogDisable,		/**< Watchdog Disable state, after watchdog has expired */
	stateLowBatDisable			/**< Low Bat Disable state, when battery is too low */
}  APPLICATION_State;

/** Structure containing information about current system state */
typedef struct {
	APPLICATION_State state;	/**< Store current application state*/
	uint8_t cmd;				/**< Current received command, CMD_NONE if no command was received */
	uint16_t batteryState;		/**< Last battery message received from battery driver*/
	char batteryUpdate;			/**< Battery state has changed and need to be processed*/
	char inCharge;				/**< Robot is currently plugged for charging*/
	int32_t distance;			/**< Distance of movement requested with a MOVE command*/
	int32_t turns;				/**< Number of turn requested with a TURN command*/
	int32_t motor_left;			/**< Speed to be applied for left motor */
	int32_t motor_right;		/**< Speed to be applied for right motor*/
	char endOfMouvement;		/**< Flag indicating last movement request has ended, ready for new movement*/
	char powerOffRequired;		/**< Flag indicating system power off*/
	uint16_t senderAddress;		/**< Xbee sender address (not used)*/
	uint8_t rfProblem;			/**< Xbee RF quality (not used)*/
} APPLICATION_Infos;

/** Structure storing counters used for watchdog and system inactivity.
 * Used notably to check if watchdog reset was missed or power down system because of inactivity */
typedef struct {
	uint32_t startupCnt;		/**< Counter used during wake up, to allow couple of second for
	                                 battery animation to show up before system enters IDLE state*/
	uint32_t inactivityCnt;		/**< Counter used to check system inactivity (no command received)*/
	uint32_t watchdogCnt;		/**< Counter used for watchdog check. Reset when RESET_WATCHDOG command is received */
	char watchdogEnabled;		/**< Flag used to know if watchdog is enabled or not*/
	char watchdogMissedCnt;		/**< Counter used to store each time watchdog reset is missed*/
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

void APPLICATION_Thread(void* params);
void APPLICATION_StateMachine();
void APPLICATION_PowerOff();
void APPLICATION_TransitionToNewState(APPLICATION_State new_state);

APPLICATION_Infos systemInfos = {0};
APPLICATION_Timeout systemTimeout = {0};

/**
  * @brief  Initialization of drivers, modules and application.
  * @param  None
  * @return None
  */
void APPLICATION_Init(void) {
	/* Init des messages box */
	MESSAGE_Init();

	/* Init de l'afficheur */
	LEDS_Init();

	/* Init de la partie RF / reception des messages */
	XBEE_Init();
	BATTERY_Init();
	MOTORS_Init();

	/* Create the task without using any dynamic memory allocation. */
	xHandleApplicationMain = xTaskCreateStatic(
			APPLICATION_Thread,       /* Function that implements the task. */
			"APPLICATION Thread",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityApplicationHandler,/* Priority at which the task is created. */
			xStackApplicationMain,          /* Array to use as the task's stack. */
			&xTaskApplicationMain);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleApplicationMain);

	/* Create a periodic task without using any dynamic memory allocation. */
	xHandleTimerTimeout = xTimerCreateStatic(
			"Counters Timer",
			pdMS_TO_TICKS(APPLICATION_COUNTERS_DELAY),
			pdTRUE,
			( void * ) 0,
			vTimerTimeoutCallback,
			&xBufferTimerTimeout);
	xTimerStart(xHandleTimerTimeout,0 );
}

/**
  * @brief  Application thread (main thread)
  *
  * This thread mainly waits for messages from others threads or drivers, store informations, set various flags
  * and then call state machine function (APPLICATION_StateMachine()) for processing actions.
  *
  * @param[in] params startup parameters for task (not used)
  * @return None
  */
void APPLICATION_Thread(void* params) {
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

				if (decodedCmd->type==CMD_NONE)
					cmdSendAnswer(ANS_UNKNOWN);
				else if (decodedCmd->type == CMD_INVALID_CHECKSUM)
					cmdSendAnswer(ANS_ERR);
				else {
					systemInfos.cmd = decodedCmd->type;
					systemTimeout.inactivityCnt = 0;

					/* Manage answer to command, when possible
					 * (further treatment of the command will be done in APPLICATION_StateMachine) */
					switch (decodedCmd->type) {
					case CMD_PING:
					case CMD_TEST:
					case CMD_DEBUG:
						cmdSendAnswer(ANS_OK);
					case CMD_POWER_OFF:
						systemInfos.powerOffRequired=1;
						cmdSendAnswer(ANS_OK);
						break;
					case CMD_GET_BATTERY:
						cmdSendBatteryLevel(systemInfos.batteryState);
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
				}

				free(decodedCmd);
				free(receivedCMD);
				break;

		default:
			break;
			}

			break;

		case MSG_ID_BAT_ADC_ERR:
			/* depart en panic: error 2 */
			PANIC_Raise(panic_adc_err);
			break;
		case MSG_ID_BAT_CHARGE_ERR:
			/* depart en panic: error 3 */
			PANIC_Raise(panic_charger_err);
			break;
		case MSG_ID_BAT_CHARGE_COMPLETE:
		case MSG_ID_BAT_CHARGE_LOW:
		case MSG_ID_BAT_CHARGE_MED:
		case MSG_ID_BAT_CHARGE_HIGH:
			systemInfos.batteryUpdate=1;
			systemInfos.inCharge=1;
			systemInfos.batteryState = msg.id;
			break;
		case MSG_ID_BAT_CRITICAL_LOW:
		case MSG_ID_BAT_LOW:
		case MSG_ID_BAT_MED:
		case MSG_ID_BAT_HIGH:
			systemInfos.batteryUpdate=1;
			systemInfos.inCharge=0;
			systemInfos.batteryState = msg.id;
			break;
		case MSG_ID_MOTORS_END_OF_MOUVMENT:
			systemInfos.endOfMouvement= 1;
			break;

		case MSG_ID_BUTTON_PRESSED:
			systemInfos.powerOffRequired =1;
			break;
		}

		APPLICATION_StateMachine();
	}
}

/**
  * @brief  State machine processing function
  *
  * This function processes received messages depending on current system state.
  * In case of state transition, function APPLICATION_TransitionToNewState will be called at end for transition and clean up
  *
  * @param  None
  * @return None
  */
void APPLICATION_StateMachine(void) {
	LEDS_State ledState = leds_off;

	if (systemInfos.powerOffRequired)
		APPLICATION_PowerOff(); // system will halt here

	if ((systemInfos.inCharge) && (systemInfos.state != stateInCharge)) {
		APPLICATION_TransitionToNewState(stateInCharge);
	}

	if (systemInfos.batteryUpdate) {
		if (systemInfos.batteryState==MSG_ID_BAT_CRITICAL_LOW) {
			ledState = leds_bat_critical_low;
			APPLICATION_TransitionToNewState(stateLowBatDisable);

			LEDS_Set(ledState);
		} else if (systemInfos.state == stateInCharge) {
			switch (systemInfos.batteryState) {
			case MSG_ID_BAT_CHARGE_COMPLETE:
				ledState= leds_bat_charge_complete;
				break;
			case  MSG_ID_BAT_CHARGE_HIGH:
				ledState= leds_bat_charge_high;
				break;
			case  MSG_ID_BAT_CHARGE_MED:
				ledState= leds_bat_charge_med;
				break;
			case  MSG_ID_BAT_CHARGE_LOW:
				ledState= leds_bat_charge_low;
				break;
			}

			LEDS_Set(ledState);
		} else if (systemInfos.state==stateStartup) {
			switch (systemInfos.batteryState) {
			case  MSG_ID_BAT_HIGH:
				ledState= leds_bat_high;
				break;
			case  MSG_ID_BAT_MED:
				ledState= leds_bat_med;
				break;
			case  MSG_ID_BAT_LOW:
				ledState= leds_bat_low;
				break;
			}

			LEDS_Set(ledState);
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
					(systemInfos.state == stateInMouvement) ||
					(systemInfos.state == stateWatchdogDisable)) {
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

				if (systemInfos.cmd == CMD_START_WITH_WATCHDOG) {
					systemTimeout.watchdogEnabled = 1;
					systemTimeout.watchdogCnt=0;
					systemTimeout.watchdogMissedCnt=0;
				}

				APPLICATION_TransitionToNewState(stateRun);
			} else
				cmdSendAnswer(ANS_ERR);
			break;
		case CMD_RESET_WATCHDOG:
			if ((systemInfos.state == stateRun) || (systemInfos.state == stateInMouvement)) {
				if ((systemTimeout.watchdogEnabled ==0 ) ||
						((systemTimeout.watchdogCnt>=(APPLICATION_WATCHDOG_MIN/100)) && (systemTimeout.watchdogCnt<=(APPLICATION_WATCHDOG_MAX/100)))) {
					systemTimeout.watchdogMissedCnt=0; // gg, watchog is reset correctly
					cmdSendAnswer(ANS_OK);
				} else {
					systemTimeout.watchdogMissedCnt++; // If you reset at the wrong time, it still count as missed watchdog reset
					cmdSendAnswer(ANS_ERR);
				}

				systemTimeout.watchdogCnt =0;
			} else
				cmdSendAnswer(ANS_ERR);
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
				} else { // If MOVE and TURN cmds come with parameters, reject them
					cmdSendAnswer(ANS_ERR);
				}
			} else
				cmdSendAnswer(ANS_ERR);
			break;
		default: // commands no common for every states
			break;
		}
	}

	if ((systemInfos.state==stateInMouvement) && (systemInfos.endOfMouvement)) {
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
	systemInfos.cmd = CMD_NONE;
	systemInfos.endOfMouvement =0;
	systemInfos.powerOffRequired=0;
}

/**
  * @brief  State machine transition clean up
  *
  * This function is part of statemachine processing. It's job is to process and cleanup statemachine transition.
  *
  * @param[in] new_state New state to apply to system
  * @return None
  */
void APPLICATION_TransitionToNewState(APPLICATION_State new_state) {
	LEDS_State ledState = leds_off;

	switch (new_state) {
	case stateStartup:
		// nothing to do here
		break;
	case stateIdle:
		ledState = leds_idle;
		LEDS_Set(ledState);

		MOTORS_Stop();
		systemTimeout.inactivityCnt=0;
		systemTimeout.watchdogEnabled=0;
		break;
	case stateRun:
		if (systemTimeout.watchdogEnabled)
			ledState = leds_run_with_watchdog;
		else
			ledState = leds_run;

		LEDS_Set(ledState);

		MOTORS_Stop();
		break;
	case stateInMouvement:
		ledState = leds_run;
		LEDS_Set(ledState);

		if (systemInfos.cmd == CMD_MOVE) {
			MOTORS_Move( systemInfos.distance);
		} else { /* obviously, cmd is CMD_TURN */
			MOTORS_Turn(systemInfos.turns);
		}
		break;
	case stateInCharge:
		/* les leds sont gerées dans APPLICATION_StateMachine */
		MOTORS_Stop();
		systemTimeout.watchdogEnabled=0;
		break;
	case stateWatchdogDisable:
		ledState = leds_watchdog_expired;
		LEDS_Set(ledState);

		systemTimeout.watchdogEnabled=0;
		break;
	case stateLowBatDisable:
		ledState = leds_bat_critical_low;
		LEDS_Set(ledState);

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

/**
  * @brief  Power off robot
  *
  * Disable main regulator and power off system. Used after inactivity or when user press on/off button.
  *
  * @param  None
  * @return None
  */
void APPLICATION_PowerOff(void) {
	/*
	 * TODO: a decommenter quand le code sera debuggé
	 */
	HAL_GPIO_WritePin(SHUTDOWN_GPIO_Port, SHUTDOWN_Pin, GPIO_PIN_RESET);

	while (1){
		__WFE(); // Attente infinie que le regulateur se coupe.
	}
}

/**
  * @brief  Periodic task used for system counter update
  *
  * This periodic task is called every 100 ms and is used for updating inactivity, startup and watchdog counters,
  * sending messages or triggering transition if necessary.
  *
  * @remark Time constants are expressed in ms, thus explaining the division by 100 used in comparison.
  *
  * @param[in]  xTimer Handler for periodic task
  * @return None
  */
void vTimerTimeoutCallback( TimerHandle_t xTimer ) {
	if (systemInfos.state == stateStartup) {
		systemTimeout.startupCnt++;
		if (systemTimeout.startupCnt++>=(APPLICATION_STARTUP_DELAY/100))
			APPLICATION_TransitionToNewState(stateIdle);
	}

	if (systemInfos.state != stateInCharge) {
		systemTimeout.inactivityCnt++;
		if (systemTimeout.inactivityCnt>=(APPLICATION_INACTIVITY_TIMEOUT/100))
			/* send a message Button_Pressed to enable power off */
			MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_BUTTON_PRESSED, APPLICATION_Mailbox, (void*)NULL);

		if (systemTimeout.watchdogEnabled) {
			systemTimeout.watchdogCnt++;

			if (systemTimeout.watchdogCnt>(APPLICATION_WATCHDOG_MAX/100)) {
				systemTimeout.watchdogCnt=0;
				systemTimeout.watchdogMissedCnt++;
			}

			if (systemTimeout.watchdogMissedCnt>=APPLICATION_WATCHDOG_MISSED_MAX)
				APPLICATION_TransitionToNewState(stateWatchdogDisable); /* TODO: Reprendre pour en faire un envoi de message */
		}
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
