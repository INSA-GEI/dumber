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

CMD_Generic* cmdDecode(char* cmd, uint8_t length) {
	CMD_Generic* decodedCmd;
	char cmd_type = cmd[0];
	int val;
	uint32_t startTime, endTime;
	volatile uint32_t duration;

	startTime = SysTick->VAL;

	switch (cmd_type)
	{
	case CMD_MOVE:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Move));
		decodedCmd->type = cmd[0];
		{
			char rawCmd[length+1];
			memcpy((void*)rawCmd, (void*)cmd, length);
			rawCmd[length] = (char)0; /* 0 ending string */

			sscanf (&rawCmd[1], "%d", &val);

			((CMD_Move*)decodedCmd)->distance= (int16_t) val;
		}
		break;

	case CMD_TURN:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Turn));
		decodedCmd->type = cmd[0];
		{
			char rawCmd[length+1];
			memcpy((void*)rawCmd, (void*)cmd, length);
			rawCmd[length] = (char)0; /* 0 ending string */

			sscanf (&rawCmd[1], "%d", &val);

			((CMD_Turn*)decodedCmd)->turns = (int16_t) val;
			((CMD_Turn*)decodedCmd)->turns = ((CMD_Turn*)decodedCmd)->turns * 1.4;
		}
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
		break;
	default:
		decodedCmd = CMD_DECODE_UNKNOWN;
	}

	endTime=SysTick->VAL;

	if (endTime>startTime)
		duration = (uint32_t)(SysTick->LOAD+1)-endTime+startTime;
	else
		duration = startTime-endTime;

	return decodedCmd;
}

void cmdSendAnswer(uint16_t address, uint8_t ans) {
	ANS_Generic answer;

	answer.ans = ans;
	XBEE_SendData(address, (char*)&answer, sizeof (answer));
}

void cmdSendBatteryLevel(uint16_t address, char level) {
	ANS_Battery answer;
	char localLevel=level;

	if (localLevel<0) localLevel=0;
	else if (localLevel>2) localLevel=2;

	answer.ans = ANS_OK;
	answer.bat_level = localLevel;

	XBEE_SendData(address, (char*)&answer, sizeof (answer));
}

void cmdSendVersion(uint16_t address) {
	ANS_Version answer;

	answer.ans = ANS_OK;
	strcpy (answer.version, SYSTEM_VERSION_STR);

	XBEE_SendData(address, (char*)&answer, sizeof (answer));
}

void cmdSendBusyState(uint16_t address, uint8_t state) {
	ANS_Busy_State answer;

	answer.ans = ANS_OK;
	answer.state = state;

	if (answer.state >1) answer.state=0;

	XBEE_SendData(address, (char*)&answer, sizeof (answer));
}
