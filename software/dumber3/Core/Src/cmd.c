/*
 * cmd.c
 *
 *  Created on: 3 oct. 2022
 *      Author: dimercur
 */

#include "cmd.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Definition des commandes */

//void cmdAddChecksum(char* ans) {
//	uint16_t j;
//	unsigned char checksum=0;
//
//	for (j = 0; ans[j] != END_OF_CMD; j++)
//		checksum ^= ans[j];
//	if (checksum == END_OF_CMD)
//		checksum++;
//	ans[j] = checksum;
//	ans[j + 1] = END_OF_CMD;
//}

/**
 * @brief 		Verifie le checksum de la commande recue
 *
 * 				Vérifie le dernier carctére de receiptString sensé être le checksum.
 *				Si celui-ci est bon, ll retournera 0 et supprimera le checksum du tableau receiptString
 * 				sinon il retournera 1 sans faire de modification.
 * @param  		None
 * @retval 		0 ou 1
 *
 */
ANS_Type cmdVerifyChecksum(char* cmd) {
	uint16_t j;
	uint16_t length;
	unsigned char checksum=0;

	length = strlen(cmd);
	for (j = 0; j < length - 2; j++) {
		checksum ^= cmd[j];
	}
	if (checksum == END_OF_CMD)
		checksum++;

	if (cmd[j] == checksum) {
		cmd[length - 2] = 0;
		cmd[length - 1] = 0;
		cmd[length] = 0;

		return okANS;
	} else
		return errANS;
}

ANS_Type cmdIsValid(char* cmd) {
	uint16_t j;
	uint16_t length;
	unsigned char checksum=0;

	length = strlen(cmd);
	for (j = 0; j < length - 2; j++) {
		checksum ^= cmd[j];
	}
	if (checksum == END_OF_CMD)
		checksum++;

	if (cmd[j] == checksum)
		return okANS;
	else
		return errANS;
}

CMD_Generic* cmdDecode(char* cmd) {
	CMD_Generic* decodedCmd;

	if (cmdVerifyChecksum(cmd) != okANS)
		return CMD_DECODE_INVALID;
	else if (cmd[0] == moveCMD) {
		int32_t laps;
		uint16_t testReception = sscanf(cmd, "M=%li", &laps);

		if (testReception!=1) return CMD_DECODE_INVALID;
		else {
			decodedCmd=(CMD_Generic*)malloc(sizeof(CMD_Move));
			((CMD_Move*)decodedCmd)->type = moveCMD;
			((CMD_Move*)decodedCmd)->distance = laps;
		}
	} else if (cmd[0] == turnCMD) {
		int32_t degree;
		uint16_t testReception = sscanf(cmd, "T=%li", &degree);

		if (testReception != 1) return CMD_DECODE_INVALID;
		else {
			degree = degree * 1.40;
			decodedCmd=(CMD_Generic*)malloc(sizeof(CMD_Turn));
			((CMD_Turn*)decodedCmd)->type = moveCMD;
			((CMD_Turn*)decodedCmd)->turns= degree;
		}
	} else if (cmd[0] == setMotorCMD) {
		int32_t moteurG,moteurD;
		uint16_t testReception = sscanf(cmd, "m=%li,%li", &moteurG, &moteurD);

		if (testReception != 1) return CMD_DECODE_INVALID;
		else {
			decodedCmd=(CMD_Generic*)malloc(sizeof(CMD_SetMotor));
			((CMD_SetMotor*)decodedCmd)->type = moveCMD;
			((CMD_SetMotor*)decodedCmd)->motor_left= moteurG;
			((CMD_SetMotor*)decodedCmd)->motor_right= moteurD;
		}
	} else if ((cmd[0] == pingCMD) || (cmd[0] == resetCMD) || (cmd[0] == startWWatchDogCMD) || (cmd[0] == resetWatchdogCMD) ||
			(cmd[0] == getBatteryVoltageCMD) || (cmd[0] == getVersionCMD) || (cmd[0] == startWithoutWatchCMD) || (cmd[0] == busyStateCMD) ||
			(cmd[0] == testCMD) || (cmd[0] == debugCMD) || (cmd[0] == powerOffCMD)) {
		decodedCmd=(CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = cmd[0];
	} else return CMD_DECODE_UNKNOWN;

	return decodedCmd;
}

void cmdSendAnswer(ANS_Type ans) {
	char cmdStr[4];
	cmdStr[0] = (char)ans;
	cmdStr[1] = (char)ans;
	cmdStr[2] = END_OF_CMD;
	cmdStr[3] = 0; // end of string
	XBEE_SendData(cmdStr, 3);
}

void cmdSendBatteryLevel(char level) {
	char cmdStr[4];
	char localLevel=level;

	if (localLevel<0) localLevel=0;
	else if (localLevel>2) localLevel=2;

	cmdStr[0] = localLevel+'0';
	cmdStr[1] = localLevel+'0';
	cmdStr[2] = END_OF_CMD;
	cmdStr[3] = 0; // end of string
	XBEE_SendData(cmdStr, 3);
}

void cmdSendVersion() {
	XBEE_SendData(SYSTEM_VERSION_STR, strlen(SYSTEM_VERSION_STR));
}

