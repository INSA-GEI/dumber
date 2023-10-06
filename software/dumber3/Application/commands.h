/*
 * cmd.h
 *
 *  Created on: 3 oct. 2022
 *      Author: dimercur
 */

#ifndef INC_CMD_H_
#define INC_CMD_H_

#include "application.h"

typedef enum {
	CMD_NONE=0x0,
	CMD_PING,
	CMD_RESET,
	CMD_START_WITH_WATCHDOG,
	CMD_RESET_WATCHDOG,
	CMD_GET_BATTERY,
	CMD_GET_VERSION,
	CMD_START_WITHOUT_WATCHDOG,
	CMD_MOVE,
	CMD_TURN,
	CMD_GET_BUSY_STATE,
	CMD_TEST,
	CMD_DEBUG,
	CMD_POWER_OFF
} CMD_CommandsType;

typedef enum {
	ANS_OK=0x80,
	ANS_ERR,
	ANS_UNKNOWN
} CMD_AnswersType;

typedef enum {
	ANS_BAT_EMPTY=0,
	ANS_BAT_LOW,
	ANS_BAT_OK
} CMD_BatteryLevelType;

typedef enum {
	ANS_STATE_NOT_BUSY=0,
	ANS_STATE_BUSY
} CMD_BusyType;

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
	uint16_t version;
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

CMD_Generic* cmdDecode(char* cmd, uint8_t length);
void cmdSendAnswer(uint8_t ans);
void cmdSendString(char* str);
void cmdSendBatteryLevel(char level);
void cmdSendVersion(void);
void cmdSendBusyState(uint8_t state);

#endif /* INC_CMD_H_ */
