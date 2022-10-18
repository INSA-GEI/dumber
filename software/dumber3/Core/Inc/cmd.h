/*
 * cmd.h
 *
 *  Created on: 3 oct. 2022
 *      Author: dimercur
 */

#ifndef INC_CMD_H_
#define INC_CMD_H_

#include "main.h"

typedef enum {
	pingCMD 				= 'p',
	resetCMD 				= 'r',
	setMotorCMD 			= 'm',
	startWWatchDogCMD 		= 'W',
	resetWatchdogCMD 		= 'w',
	getBatteryVoltageCMD 	= 'v',
	getVersionCMD  			= 'V',
	startWithoutWatchCMD 	= 'u',
	moveCMD  				= 'M',
	turnCMD  				= 'T',
	busyStateCMD 			= 'b',
	testCMD  				= 't',
	debugCMD  				= 'a',
	powerOffCMD 			= 'z'
} CMD_Type;

typedef enum {
	okANS              		= 'O',
	errANS             		= 'E',
	unknownANS         		= 'C',
	batOK              		= '2',
	batLOW             		= '1',
	batEMPTY           		= '0'
} ANS_Type;

#define END_OF_CMD			'\r'

typedef struct {
	CMD_Type type;
} CMD_Generic;

typedef struct {
	CMD_Type type;
	int32_t motor_left;
	int32_t motor_right;
} CMD_SetMotor;

typedef struct {
	CMD_Type type;
	int32_t distance;
} CMD_Move;

typedef struct {
	CMD_Type type;
	int32_t turns;
} CMD_Turn;

#define CMD_DECODE_INVALID		((CMD_Generic*)NULL)
#define CMD_DECODE_UNKNOWN		((CMD_Generic*)UINT32_MAX)

ANS_Type cmdIsValid(char* cmd);
CMD_Generic* cmdDecode(char* cmd);
void cmdSendAnswer(ANS_Type ans);
void cmdSendBatteryLevel(char level);
void cmdSendVersion();

#endif /* INC_CMD_H_ */
