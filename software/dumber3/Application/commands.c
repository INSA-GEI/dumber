/*
 * cmd.c
 *
 *  Created on: 3 oct. 2022
 *      Author: dimercur
 */

#include "commands.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Definition des commandes */

CMD_Generic* cmdDecode(char* cmd) {
	CMD_Generic* decodedCmd;

	switch (cmd[0])
	{
	case CMD_MOVE:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Move));
		((CMD_Move*)decodedCmd)->distance = ((uint16_t)cmd[1]<<8) + (uint16_t)cmd[2];
		break;

	case CMD_TURN:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Turn));

		((CMD_Turn*)decodedCmd)->turns = ((uint16_t)cmd[1]<<8) + (uint16_t)cmd[2];
		((CMD_Turn*)decodedCmd)->turns = ((CMD_Turn*)decodedCmd)->turns * 1.4;

		break;

	case CMD_PING:
	case CMD_RESET:
	case CMD_START_WITHOUT_WATCHDOG:
	case CMD_START_WITH_WATCHDOG:
	case CMD_RESET_WATCHDOG:
	case CMD_GET_BATTERY:
	case CMD_GET_BUSY_STATE:
	case CMD_GET_VERSION:
	case CMD_TEST:
	case CMD_DEBUG:
	case CMD_POWER_OFF:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = cmd[0];

	default:
		decodedCmd = CMD_DECODE_UNKNOWN;
	}

	return decodedCmd;
}

void cmdSendAnswer(uint8_t ans) {
	ANS_Generic answer;

	answer.ans = ans;
	XBEE_SendData((char*)&answer, sizeof (answer));
}

void cmdSendBatteryLevel(char level) {
	ANS_Battery answer;
	char localLevel=level;

	if (localLevel<0) localLevel=0;
	else if (localLevel>2) localLevel=2;

	answer.ans = ANS_OK;
	answer.bat_level = localLevel;

	XBEE_SendData((char*)&answer, sizeof (answer));
}

void cmdSendVersion() {
	ANS_Version answer;

	answer.ans = ANS_OK;
	answer.version = SYSTEM_VERSION;

	XBEE_SendData((char*)&answer, sizeof (answer));
}

void cmdBusyState(uint8_t state) {
	ANS_Busy_State answer;

	answer.ans = ANS_OK;
	answer.state = state;

	if (answer.state >1) answer.state=0;

	XBEE_SendData((char*)&answer, sizeof (answer));
}
