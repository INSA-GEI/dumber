/*
 * xbee.h
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#ifndef INC_XBEE_H_
#define INC_XBEE_H_

#include "main.h"

void XBEE_Init(void);
int XBEE_SendData(char* data, int length);

#endif /* INC_XBEE_H_ */
