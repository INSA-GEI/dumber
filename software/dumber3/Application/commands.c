/*
 * cmd.c
 *
 *  Created on: 3 oct. 2022
 *      Author: dimercur
 */

#include "commands.h"
#include <stdlib.h>
#include <string.h>

/* Definition des commandes */

#define PingCMD                 'p'
#define ResetCMD                'r'
#define SetMotorCMD             'm'
#define StartWWatchDogCMD       'W'
#define ResetWatchdogCMD        'w'
#define GetBatteryVoltageCMD    'v'
#define GetVersionCMD           'V'
#define StartWithoutWatchCMD    'u'
#define MoveCMD                 'M'
#define TurnCMD                 'T'
#define BusyStateCMD            'b'
#define TestCMD                 't'
#define DebugCMD                'a'
#define PowerOffCMD             'z'

#define OK_ANS                  "O\r"
#define ERR_ANS                 "E\r"
#define UNKNOW_ANS              "C\r"
#define BAT_OK                  "2\r"
#define BAT_LOW                 "1\r"
#define BAT_EMPTY               "0\r"

char* cmdAddChecksum(const char* str);
char cmdVerifyChecksum(char* str);

/** @addtogroup Checksum
 * @{
 */
/**
 * @brief 		Inclut le checksum à sendString
 *
 *				Parcours str pour y calculer le checksum ( xor sur chaque caractére)
 *				et inclut le resultat en fin de chaine.
 *
 * @param  		string sans checksum
 * @retval 		string avec checksum
 *
 */
char* cmdAddChecksum(const char* str) {
	uint16_t j;
	unsigned char checksum=0;
	char *outstr = (char*) malloc(strlen(str)+2); // +1 for checksum to add, +1 for zero ending

	for (j = 0; str[j] != '\r'; j++) {
		outstr[j] = str[j];
		checksum ^= str[j];
	}

	if (checksum == '\r')
		checksum++;
	outstr[j] = checksum;
	outstr[j + 1] = '\r';
	outstr[j + 2] = 0;

	return outstr;
}

/**
 * @brief 		Verifie le checksum de la variable global recepitString
 *
 * 				Vérifie le dernier carctére de str sensé être le checksum.
 *				Si celui-ci est bon, ll retournera 0 et supprimera le checksum de str
 * 				sinon il retournera 1 sans faire de modification.
 * @param  		None
 * @retval 		0 ou 1
 *
 */
char cmdVerifyChecksum(char* str) {
	uint16_t j;
	uint16_t length;
	unsigned char checksum=0;

	length = strlen(str);
	for (j = 0; j < length - 2; j++) {
		checksum ^= str[j];
	}
	if (checksum == '\r')
		checksum++;

	if (str[j] == checksum) {
		str[length - 2] = 13;
		str[length - 1] = 0;
		str[length] = 0;

		return 0;
	} else
		return 1;
}

/**
 * @}
 */

//CMD_Generic* cmdDecode(char* cmd, uint8_t length) {
//	CMD_Generic* decodedCmd;
//	char cmd_type = cmd[0];
//
//	switch (cmd_type)
//	{
//	case CMD_MOVE:
//		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Move));
//		decodedCmd->type = CMD_MOVE;
//		((CMD_Move*)decodedCmd)->distance = ((int16_t)cmd[1]<<8) + (int16_t)cmd[2];
//		break;
//
//	case CMD_TURN:
//		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Turn));
//		decodedCmd->type = CMD_TURN;
//		((CMD_Turn*)decodedCmd)->turns = ((int16_t)cmd[1]<<8) + (int16_t)cmd[2];
//		break;
//
//	case CMD_PING:
//	case CMD_RESET:
//	case CMD_START_WITHOUT_WATCHDOG:
//	case CMD_START_WITH_WATCHDOG:
//	case CMD_RESET_WATCHDOG:
//	case CMD_GET_BATTERY:
//	case CMD_GET_BUSY_STATE:
//	case CMD_GET_VERSION:
//	case CMD_TEST:
//	case CMD_DEBUG:
//	case CMD_POWER_OFF:
//		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
//		decodedCmd->type = cmd_type;
//		break;
//
//	default:
//		decodedCmd = CMD_DECODE_UNKNOWN;
//	}
//
//	return decodedCmd;
//}

CMD_Generic* cmdDecode(char* cmd, uint8_t length) {
	CMD_Generic* decodedCmd;
	char cmd_type = cmd[0];

	switch (cmd_type)
	{
	case MoveCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Move));
		decodedCmd->type = CMD_MOVE;
		((CMD_Move*)decodedCmd)->distance = ((int16_t)cmd[1]<<8) + (int16_t)cmd[2];
		break;

	case TurnCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Turn));
		decodedCmd->type = CMD_TURN;
		((CMD_Turn*)decodedCmd)->turns = ((int16_t)cmd[1]<<8) + (int16_t)cmd[2];
		break;

	case PingCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_PING;
		break;
	case ResetCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_RESET;
		break;
	case StartWWatchDogCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_START_WITH_WATCHDOG;
		break;
	case StartWithoutWatchCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_START_WITHOUT_WATCHDOG;
		break;
	case ResetWatchdogCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_RESET_WATCHDOG;
		break;
	case GetBatteryVoltageCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_GET_BATTERY;
		break;
	case GetVersionCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_GET_VERSION;
		break;
	case BusyStateCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_GET_BUSY_STATE;
		break;
	case TestCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_TEST;
		break;
	case DebugCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_DEBUG;
		break;
	case PowerOffCMD:
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_POWER_OFF;
		break;

	default:
		decodedCmd = CMD_DECODE_UNKNOWN;
	}

	return decodedCmd;
}

void cmdSendAnswer(uint8_t ans) {
	char* answer;

	switch (ans) {
	case ANS_OK:
		answer = cmdAddChecksum(OK_ANS);
		break;
	case ANS_ERR:
		answer = cmdAddChecksum(ERR_ANS);
		break;
	default:
		answer = cmdAddChecksum(UNKNOW_ANS);
		break;
	}

	MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (char*) answer);
}

void cmdSendString(char *str) {
	char* answer;
	int strlength = strlen(str);
	char localstr[strlength+2];

	strncpy(localstr, str, strlength+2);
	if (localstr[strlength-1]!='\r') { // \r n'est pas present
		localstr[strlength]='\r';
		localstr[strlength+1]=0;
	}

	answer = cmdAddChecksum(localstr);
	MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (char*) answer);
}

void cmdSendBatteryLevel(char level) {
	char* answer;
	char localLevel=level;

	if (localLevel<0) localLevel=0;
	else if (localLevel>2) localLevel=2;

	switch (localLevel) {
	case 2:
		answer = cmdAddChecksum("2\r");
		break;
	case 1:
		answer = cmdAddChecksum("1\r");
		break;
	default:
		answer = cmdAddChecksum("0\r");
		break;
	}

	MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (char*) answer);
}

void cmdSendVersion(void) {
	int versionlength = strlen(SYSTEM_VERSION_STR);
	char versionstr[versionlength+2];
	char* answer;

	strncpy(versionstr,SYSTEM_VERSION_STR,versionlength+2);
	versionstr[versionlength] = '\r';
	versionstr[versionlength+1]=0; // zero ending string

	answer = cmdAddChecksum(versionstr);

	MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (char*) answer);
}

void cmdSendBusyState(uint8_t state) {
	char* answer;

	if (state)
		answer = cmdAddChecksum("1\r");
	else
		answer = cmdAddChecksum("0\r");

	MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (char*) answer);
}
