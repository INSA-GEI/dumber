/*
 * rtos_support.h
 *
 *  Created on: Jan 12, 2024
 *      Author: dimercur
 */

#ifndef RTOS_SUPPORT_H_
#define RTOS_SUPPORT_H_

#include "application.h"

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup RTOS_SUPPORT
 * @{
 */

/** @addtogroup RTOS_SUPPORT_Public Public
 * @{
 */

void RTOS_SUPPORT_Init(void);
uint32_t RTOS_SUPPORT_GetTimer(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* RTOS_SUPPORT_H_ */
