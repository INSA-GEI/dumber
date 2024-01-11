/**
 ******************************************************************************
 * @file motors.h
 * @brief motors driver header
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

#ifndef INC_MOTORS_H_
#define INC_MOTORS_H_

#include "application.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup MOTORS
  * @{
  */

/** @addtogroup MOTORS_Public Public
  * @{
  */

void MOTORS_Init(void);
void MOTORS_Move(int32_t distance);
void MOTORS_Turn(int32_t tours);
void MOTORS_Stop(void);

void MOTORS_TimerEncodeurUpdate (TIM_HandleTypeDef *htim);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* INC_MOTORS_H_ */
