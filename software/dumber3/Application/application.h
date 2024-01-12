/**
 ******************************************************************************
 * @file application.h
 * @brief application header
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
#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "cmsis_os.h"

#include "config.h"

#include "messages.h"
#include "leds.h"
#include "battery.h"
#include "motors.h"
#include "xbee.h"
#include "commands.h"

#include "rtos_support.h"

#include "main.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup APPLICATION
  * @{
  */

/** @addtogroup APPLICATION_Public Public
  * @{
  */

void APPLICATION_Init(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* INC_APPLICATION_H_ */
