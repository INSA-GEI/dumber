#ifndef Battery_H
#define Battery_H

#include "stm32f10x.h"

 

extern __IO uint16_t ADCConvertedValue[16];
void MAP_batteryPin(void);
void DMA_BAT(void);
void ADC1_CONFIG(void);
void INIT_IT_DMA(void);
void startACQDMA(void);
void voltagePrepare(void);

#endif /* Battery_H */

