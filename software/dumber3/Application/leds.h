/**
 ******************************************************************************
 * @file leds.h
 * @brief leds driver header
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

#ifndef INC_LEDS_H_
#define INC_LEDS_H_

#include "application.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup LEDS
 * @{
 */

/** @addtogroup LEDS_Public Public
 * @{
 */

/** Enumeration class defining possible leds animations */
typedef enum {
	leds_off=0,					/**< No animation */
	leds_idle,					/**< Idle animation (only dot point blinking) */
	leds_run,					/**< Run animation (leds animate in circle) */
	leds_run_with_watchdog,		/**< Run with watchdog animation (leds animate in circle, with dot point blinking)  */
	leds_bat_critical_low,		/**< Critical low bat animation (C,L and B lettres) */
	leds_bat_low,				/**< Low bat animation */
	leds_bat_med,				/**< Medium charged bat animation */
	leds_bat_high,				/**< Full charged bat animation */
	leds_bat_charge_low,		/**< Charge in progress (low bat level) animation */
	leds_bat_charge_med,		/**< Charge in progress (medium bat level) animation */
	leds_bat_charge_high,		/**< Charge in progress (high bat level) animation */
	leds_bat_charge_complete,	/**< Charge complete animation */
	leds_watchdog_expired,		/**< Watchdog expired animation (squares moving) */
	leds_error_1,				/**< Error 1 animation */
	leds_error_2,				/**< Error 2 animation */
	leds_error_3,				/**< Error 3 animation */
	leds_error_4,				/**< Error 4 animation */
	leds_error_5,				/**< Error 5 animation */
	leds_state_unknown			/**< Unknown animation */
} LEDS_State;

void LEDS_Init(void);
void LEDS_Set(LEDS_State state);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* INC_LEDS_H_ */
