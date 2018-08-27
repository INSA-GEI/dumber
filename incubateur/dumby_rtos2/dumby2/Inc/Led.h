#ifndef LED_H
#define LED_H

#include "stm32f1xx_hal.h"


/*Set led color and duty-cycle value
Color can be : LED_GREEN, LED_ORANGE, LED_RED
dutyCycle is a value between 0 and 123
*/
void LED_INIT(void);
void LED_SET(uint8_t color, uint8_t dutyCycle);

#endif
