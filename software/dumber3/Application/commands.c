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
 * @retval 		0 si le checksum est faux, 1 sinon
 *
 */
char cmdVerifyChecksum(char* str) {
	uint16_t j;
	uint16_t length;
	unsigned char checksum=0;

	length = strlen(str);
	/* Warning: str should be without ending CR (0x0D) character, so, a ping command should be
	 * received as "pp", 2 chars long
	 *
	 * in the loop after, ending length is length of the caommand string whithout last char, the checksum
	 * so, we have j<length-1
	 */
	for (j = 0; j<length-1; j++) {
		checksum ^= str[j];
	}

	if (checksum == '\r')
		checksum++;

	if (str[j] == checksum) {
		/*str[length - 2] = 13;
		str[length - 1] = 0;
		str[length] = 0;*/

		str[length - 1] = 0; /* we remove checksum char */
		str[length] = 0;

		return 1;
	} else
		return 0;
}

/**
 * @}
 */

CMD_Generic* cmdDecode(char* cmd, uint8_t length) {
	CMD_Generic* decodedCmd;
	char cmd_type = cmd[0];
	char *p;

	/* First, verify checksum */
	if (cmdVerifyChecksum(cmd)) {
		switch (cmd_type)
		{
		case MoveCMD:
			decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Move));
			decodedCmd->type = CMD_MOVE;
			//((CMD_Move*)decodedCmd)->distance = ((int16_t)cmd[1]<<8) + (int16_t)cmd[2];

			/* verify that command start with "M=" */
			if ((cmd[0]=='M')&&(cmd[1]=='=')) {
				cmd = cmd+2; //cmd+2 for removing "M=" at start of the string
				((CMD_Move*)decodedCmd)->distance=strtoul(cmd , &p, 10);
				if (p==cmd)
					decodedCmd->type = CMD_NONE; /* missing number value xxxxx in "M=xxxxx" */
			} else
				decodedCmd->type = CMD_NONE; /* misformed command (should start with "M=" */
			break;

		case TurnCMD:
			decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Turn));
			decodedCmd->type = CMD_TURN;
			//((CMD_Turn*)decodedCmd)->turns = ((int16_t)cmd[1]<<8) + (int16_t)cmd[2];

			//if (!sscanf(cmd,"T=%hd",&((CMD_Turn*)decodedCmd)->turns ))
			//	decodedCmd->type = CMD_NONE;

			/* verify that command start with "T=" */
			if ((cmd[0]=='T')&&(cmd[1]=='=')) {
				cmd = cmd+2; //cmd+2 for removing "T=" at start of the string
				((CMD_Turn*)decodedCmd)->turns=strtoul(cmd , &p, 10);
				if (p==cmd)
					decodedCmd->type = CMD_NONE; /* missing number value xxxxx in "T=xxxxx" */
			} else
				decodedCmd->type = CMD_NONE; /* misformed command (should start with "T=" */
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
			decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
			decodedCmd->type = CMD_NONE;
		}
	} else { /* Checksum is wrong*/
		decodedCmd = (CMD_Generic*)malloc(sizeof(CMD_Generic));
		decodedCmd->type = CMD_INVALID_CHECKSUM;
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
