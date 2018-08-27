#ifndef MAE_H
#define MAE_H

#include "stm32f10x.h"

void machineWithStates(void);

void action_IDLE(void);
void action_RUN (void);
void action_LOWBAT(void);
void action_CHARGE(void);
#endif
