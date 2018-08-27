#ifndef XBEE_H
#define XBEE_H

#include "stm32f1xx_hal.h"

int xbeeSend(int addr_destination, char *str);
void xbee_debug_send(int address, char *str);

#endif


