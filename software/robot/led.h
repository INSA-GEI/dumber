#ifndef LED_H
#define LED_H

#include "stm32f10x.h"

#define RED 	255
#define GREEN 250

void MAP_LEDpin(void);
void INIT_TIM3Led(void);
void LEDgreen(void);
void LEDred(void);
void LEDoff(void);
void LEDorange(void);
#endif /* LED_H */
