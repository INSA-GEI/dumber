#ifndef SERVICECOM_H
#define SERVICECOM_H

#include "stm32f1xx_hal.h"
#include "common.h"
#include "cmsis_os.h"

extern osSemaphoreId synchroMesHandle;
extern char RxBuffer[TRAME_SIZE_RECEIPT];

void comInit(void);

#endif

