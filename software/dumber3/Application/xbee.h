/**
 ******************************************************************************
 * @file xbee.h
 * @brief xbee driver header
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

#ifndef INC_XBEE_H_
#define INC_XBEE_H_

#include "application.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup XBEE
  * @{
  */

/** @addtogroup XBEE_Public Public
  * @{
  */

/** Enumeration class defining status code (only used by \ref XBEE_SendData)*/
typedef enum {
	XBEE_OK=0,				/**< Function is successful */
	XBEE_CONFIG_ERROR=-1, 	/**< Configuration of XBEE gets an error (not used) */
	XBEE_TX_ACK_ERROR=-2,	/**< Tx transmission doesn't  received acknowledgment (not used in transparent mode) */
	XBEE_RX_TIMEOUT=-3,		/**< Rx frame doesn't complete in time (not used in transparent mode) */
	XBEE_RX_ERROR=-4,		/**< Rx frame error while receiving (not used in transparent mode) */
	XBEE_TX_ERROR=-5,		/**< Tx frame error while sending */
	XBEE_INVALID_FRAME=-6,	/**< Rx frame is invalid  (not used in transparent mode) */
	XBEE_TX_TIMEOUT=-7,		/**< Tx frame not sent in time */
} XBEE_Status;

/**
 * @anchor xbee_frame_type
 * @name XBEE API mode frame type
 * List of message frame used in API mode (not used now)
 */
///@{
#define XBEE_RX_PACKET_TYPE					0x90
#define XBEE_RX_EXPLICIT_TYPE				0x91
#define XBEE_TX_STATUS_TYPE					0x89
#define XBEE_AT_CMD_RESPONSE_TYPE			0x88
#define XBEE_MODEM_STATUS_TYPE				0x8A
#define XBEE_EXTENDED_TX_STATUS_TYPE		0x8B
#define XBEE_LOCAL_AT_CMD_TYPE				0x08
#define XBEE_TX_REQUEST_TYPE				0x10
#define XBEE_TX_EXPLICIT_TYPE				0x11
#define XBEE_TX_16BIT_REQUEST_TYPE			0x01
#define XBEE_RX_16BIT_PACKET_TYPE			0x81

#define XBEE_AT_STATUS_SUCCESS				0
#define XBEE_AT_STATUS_ERROR				1
#define XBEE_AT_STATUS_INVALID_COMMAND		2
#define XBEE_AT_STATUS_INVALID_PARAMETER	3

#define XBEE_TX_STATUS_SUCCESS				0x00
#define XBEE_TX_STATUS_NO_ACK				0x01
#define XBEE_TX_STATUS_CCA_FAILURE			0x02
#define XBEE_TX_STATUS_NETWORK_NO_ACK		0x21

#define XBEE_MODEM_STATUS_HW_RST			0x00
#define XBEE_MODEM_STATUS_JOINED			0x02

#define XBEE_RX_OPTIONS_ACKNOWLEDGED		0x01
#define XBEE_RX_OPTIONS_BRODCASTED			0x02
#define XBEE_RX_OPTIONS_PAN_BROADCASTED		0x04

#define XBEE_FRAME_SOF_CHAR					0x7E // '~'
///@}

/** Structure class defining received frame (only for API mode, not used now)*/
typedef struct {
	uint8_t  	type;			/**< Frame type (see \ref xbee_frame_type "XBEE API mode frame type") */
	uint16_t 	source_addr;	/**< Address of sender */
	uint8_t 	length;			/**< Length of data part */
	char  		ack;			/**< ?? */
	char		modem_status;	/**< ?? */
	char 		data[];			/**< Data buffer */
} XBEE_INCOMING_FRAME;

void XBEE_Init(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* INC_XBEE_H_ */
