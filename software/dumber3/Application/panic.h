/**
 ******************************************************************************
 * @file panic.h
 * @brief panic handler header
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

#ifndef PANIC_H_
#define PANIC_H_

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup PANIC
  * @{
  */

/** @addtogroup PANIC_Public Public
  * @{
  */

/** Enumeration class defining possible panic sources */
typedef enum {
	panic_charger_err=1,  /**< error related to battery charging */
	panic_adc_err,        /**< error related to battery voltage conversion */
	panic_malloc          /**< memory allocation failed (memory exhausted) */
} PANIC_Typedef;

void PANIC_Raise(PANIC_Typedef panicId);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* PANIC_H_ */
