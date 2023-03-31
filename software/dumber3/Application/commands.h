/*
 * cmd.h
 *
 *  Created on: 3 oct. 2022
 *      Author: dimercur
 */

#ifndef INC_CMD_H_
#define INC_CMD_H_

#include "application.h"

#define CMD_NONE 					0x0
#define CMD_PING 					0x1
#define CMD_RESET 					0x2
#define CMD_START_WITH_WATCHDOG 	0x3
#define CMD_RESET_WATCHDOG 			0x4
#define CMD_GET_BATTERY 			0x5
#define CMD_GET_VERSION 			0x6
#define CMD_START_WITHOUT_WATCHDOG 	0x7
#define CMD_MOVE 					0x8
#define CMD_TURN 					0x9
#define CMD_GET_BUSY_STATE 			0xA
#define CMD_TEST 					0xB
#define CMD_DEBUG 					0xC
#define CMD_POWER_OFF 				0xD

#define ANS_OK						0x80
#define ANS_ERR 					0x81
#define ANS_UNKNOWN 				0x82

#define ANS_BAT_OK 					0x2
#define ANS_BAT_LOW	 				0x1
#define ANS_BAT_EMPTY 				0x0

#define ANS_STATE_NOT_BUSY	 		0x0
#define ANS_STATE_BUSY 				0x1

typedef struct  __attribute__((packed)) {
	uint8_t type;
} CMD_Generic;

typedef struct  __attribute__((packed)) {
	uint8_t type;
	int16_t distance;
} CMD_Move;

typedef struct  __attribute__((packed)) {
	uint8_t type;
	int16_t turns;
} CMD_Turn;

typedef struct  __attribute__((packed)) {
	uint8_t ans;
} ANS_Generic;

typedef struct  __attribute__((packed)) {
	uint8_t ans;
	uint8_t version;
} ANS_Version;

typedef struct  __attribute__((packed)) {
	uint8_t ans;
	uint8_t bat_level;
} ANS_Battery;

typedef struct __attribute__((packed)) {
	uint8_t ans;
	uint8_t state;
} ANS_Busy_State;

#define CMD_DECODE_INVALID		((CMD_Generic*)NULL)
#define CMD_DECODE_UNKNOWN		((CMD_Generic*)UINT32_MAX)

CMD_Generic* cmdDecode(char* cmd);
void cmdSendAnswer(uint8_t ans);
void cmdSendBatteryLevel(char level);
void cmdSendVersion();
void cmdSendBusyState(uint8_t state);

#endif /* INC_CMD_H_ */
