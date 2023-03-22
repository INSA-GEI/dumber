/*
 * statemachine.h
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "cmsis_os.h"

#include "config.h"

#include "messages.h"
#include "leds.h"
#include "moteurs.h"
#include "batterie.h"
#include "xbee.h"
#include "commands.h"

#include "main.h"

void APPLICATION_Init(void);

#endif /* INC_APPLICATION_H_ */
