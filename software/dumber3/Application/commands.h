/**
 ******************************************************************************
 * @file commands.h
 * @brief commands handler header
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

#ifndef INC_CMD_H_
#define INC_CMD_H_

#include "application.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup COMMANDS
 * @{
 */

/** @addtogroup COMMANDS_Public Public
 * @{
 */

/** Enumeration class defining commands to be used in \ref CMD_Generic, \ref CMD_Move, \ref CMD_Turn structures */
typedef enum {
	CMD_NONE=0x0,				/**< Command is unknown */
	CMD_PING,					/**< PING command */
	CMD_RESET,					/**< RESET command */
	CMD_START_WITH_WATCHDOG,	/**< START_WITH_WATCHDOG command */
	CMD_RESET_WATCHDOG,			/**< RESET_WATCHDOG command */
	CMD_GET_BATTERY,			/**< GET_BATTERY command */
	CMD_GET_VERSION,			/**< GET_VERSION command */
	CMD_START_WITHOUT_WATCHDOG,	/**< START_WITHOUT_WATCHDOG command */
	CMD_MOVE,					/**< MOVE command */
	CMD_TURN,					/**< TURN command */
	CMD_GET_BUSY_STATE,			/**< GET_BUSY_STATE command */
	CMD_TEST,					/**< TEST command (not used yet) */
	CMD_DEBUG,					/**< DEBUG command (not used yet) */
	CMD_POWER_OFF,				/**< POWER_OFF command */
	CMD_INVALID_CHECKSUM=0xFF	/**< Received command string has an invalid checksum*/
} CMD_CommandsType;

/** Enumeration class defining possible generic answers */
typedef enum {
	ANS_OK=0x80,				/**< Answer OK, for a correct and processed command */
	ANS_ERR,					/**< Answer ERR, for rejected command (not accepted in current state), or invalid parameters */
	ANS_UNKNOWN					/**< Answer UNKNOWN, for unknown command */
} CMD_AnswersType;

/** Enumeration class defining battery levels */
typedef enum {
	ANS_BAT_EMPTY=0,			/**< Battery is empty, system will power off in seconds */
	ANS_BAT_LOW,				/**< Battery is low, should plug charger quickly */
	ANS_BAT_OK					/**< Charger is in correct state, charge is not required yet */
} CMD_BatteryLevelType;

/** Enumeration class defining busty states */
typedef enum {
	ANS_STATE_NOT_BUSY=0,		/**< Robot is not moving, ready for processing new movement */
	ANS_STATE_BUSY				/**< Robot is moving, not accepting new movement requests */
} CMD_BusyType;

/** Structure class used by cmdDecode function for generic commands
 *
 * @todo type of "type" field should be \ref CMD_CommandsType and not uint8_t
 */
typedef struct  __attribute__((packed)) {
	uint8_t type;				/**< Command type, as found in \ref CMD_CommandsType enum */
} CMD_Generic;

/** Structure class used by cmdDecode function for MOVE command
 *
 * @todo type of "type" field should be \ref CMD_CommandsType and not uint8_t
 */
typedef struct  __attribute__((packed)) {
	uint8_t type;				/**< Command type, as found in \ref CMD_CommandsType enum (should be \ref CMD_MOVE)*/
	int16_t distance;			/**< Distance for movement, positive for forward, negative for backward. Expressed in millimeters */
} CMD_Move;

/** Structure class used by cmdDecode function for TURN command
 *
 * @todo type of "type" field should be \ref CMD_CommandsType and not uint8_t
 */
typedef struct  __attribute__((packed)) {
	uint8_t type;				/**< Command type, as found in \ref CMD_CommandsType enum (should be \ref CMD_TURN)*/
	int16_t turns;				/**< Angle of rotation, positive for clockwise and negative for counter-clockwise. Expressed in degree */
} CMD_Turn;

CMD_Generic* cmdDecode(char* cmd, uint8_t length);
void cmdSendAnswer(uint8_t ans);
void cmdSendString(char* str);
void cmdSendBatteryLevel(uint16_t batteryState);
void cmdSendVersion(void);
void cmdSendBusyState(uint8_t state);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* INC_CMD_H_ */
