/**
 ******************************************************************************
 * @file commands.c
 * @brief commands handler body
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

#include "commands.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup COMMANDS
 * Commands handler is in charge of decoding received commands and building answer frames.
 * @{
 */

/** @addtogroup COMMANDS_Private Private
 * @{
 */

/** @name Commands definition
 *  List of accepted command identifiers
 */
///@{
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
///@}

/** @name Answers definition
 *  List of available answers
 */
///@{
#define OK_ANS                  "O\r"
#define ERR_ANS                 "E\r"
#define UNKNOW_ANS              "C\r"
#define BAT_OK                  "2\r"
#define BAT_LOW                 "1\r"
#define BAT_EMPTY               "0\r"
///@}

char* cmdAddChecksum(const char* str);
char cmdVerifyChecksum(char* str);

/**
 * @brief Add checksum to a string
 *
 *	Make a copy of a string and add checksum at its end (xor based).
 *
 * @warning This function use memory allocation for creating a copy of the original string. Be sure to free memory after use.
 *
 * @todo Error related to memory allocation (allocation failed) is not managed -> should generate a Panic
 *
 * @param[in] str string without checksum
 * @return 	string with checksum added
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
 * @brief Verify checksum of given string
 *
 * Verify if checksum of given string is correct or not. In case of success,
 * function will suppress checksum from the string, otherwise string is not modified.
 *
 * @param[in] str string to verify checksum
 * @return
 * - 0 if checksum is not correct
 * - 1 if successful
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
 * @brief Decode received command
 *
 * Command string is processed and a command structure is filled with information found in string.
 *
 * @warning This function use memory allocation for returning structure. Be sure to release memory after use.
 *
 * @param[in] cmd string with command received
 * @param[in] length length of command string
 * @return
 * - NULL if memory allocation has failed
 * - CMD_Move if string contains MOVE command
 * - CMD_Turn if string contains TURN command
 * - CMD_Generic for each other command
 *
 * Generic structure returned may contains CMD_NONE if command is unknown or CMD_INVALID_CHECKSUM if checksum is not valid
 *
 * @remark Returned value is always cast on a CMD_Generic type, but may contains CMD_Move or CMD_Turn structure. It is
 * user responsibility to check type field inside structure and cast accordingly returned value to correct structure.
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

/**
 * @brief Add correct checksum to provided answer and send it to XBEE mailbox
 *
 * @warning This function use cmdAddChecksum, so indirectly make use of memory allocation for sending message to XBEE mailbox.
 *          Be sure to release memory in XBEE driver after retrieving message from mailbox.
 *
 * @todo Error related to memory allocation (allocation failed) is not managed -> should generate a Panic
 *
 * @todo Maybe duplication between this function and cmdSendString: see if only one function with a macro/wrapper could be used
 *
 * @param[in] ans string containing answer to send, without checksum
 * @return None
 */
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

/**
 * @brief Send arbitrary answer to XBEE driver and add checksum before
 *
 * @warning This function use cmdAddChecksum, so indirectly make use of memory allocation for sending message to XBEE mailbox.
 *          Be sure to release memory in XBEE driver after retrieving message from mailbox.
 *
 * @todo Error related to memory allocation (allocation failed) is not managed -> should generate a Panic
 *
 * @todo Maybe duplication between this function and cmdSendAnswer: see if only one function with a macro/wrapper could be used
 *
 * @param[in] str string containing answer to send without checksum
 * @return None
 */
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

/**
 * @brief Send battery level answer with correct checksum to XBEE driver
 *
 * @warning This function use cmdAddChecksum, so indirectly make use of memory allocation for sending message to XBEE mailbox.
 *          Be sure to release memory in XBEE driver after retrieving message from mailbox.
 *
 * @todo Error related to memory allocation (allocation failed) is not managed -> should generate a Panic
 *
 * @todo Check batterystate type used here: seems a bit overkill (16bit for only 6 states) -> enum should be better
 *
 * @param[in] batteryState current battery state
 * @return None
 */
void cmdSendBatteryLevel(uint16_t batteryState) {
	char* answer;

	switch (batteryState) {
	case MSG_ID_BAT_CHARGE_COMPLETE:
	case MSG_ID_BAT_CHARGE_HIGH:
	case MSG_ID_BAT_HIGH:
		answer = cmdAddChecksum("2\r");
		break;
	case MSG_ID_BAT_CHARGE_MED:
	case MSG_ID_BAT_MED:
		answer = cmdAddChecksum("1\r");
		break;
	default:
		answer = cmdAddChecksum("0\r");
		break;
	}

	MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (char*) answer);
}

/**
 * @brief Send version number answer with correct checksum to XBEE driver
 *
 * @warning This function use cmdAddChecksum, so indirectly make use of memory allocation for sending message to XBEE mailbox.
 *          Be sure to release memory in XBEE driver after retrieving message from mailbox.
 *
 * @todo Error related to memory allocation (allocation failed) is not managed -> should generate a Panic
 *
 * @param None
 * @return None
 */
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

/**
 * @brief Send if robot is busy (moving) or not XBEE driver
 *
 * @warning This function use cmdAddChecksum, so indirectly make use of memory allocation for sending message to XBEE mailbox.
 *          Be sure to release memory in XBEE driver after retrieving message from mailbox.
 *
 * @todo Error related to memory allocation (allocation failed) is not managed -> should generate a Panic
 *
 * @param[in] state current robot state
 * @return None
 */
void cmdSendBusyState(uint8_t state) {
	char* answer;

	if (state)
		answer = cmdAddChecksum("1\r");
	else
		answer = cmdAddChecksum("0\r");

	MESSAGE_SendMailbox(XBEE_Mailbox, MSG_ID_XBEE_ANS, APPLICATION_Mailbox, (char*) answer);
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
