/*
 * moteurs.h
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#ifndef INC_MOTEURS_H_
#define INC_MOTEURS_H_

#include "application.h"

void MOTEURS_Init(void);
//void MOTEURS_Set(uint8_t mot, int16_t val);
//void MOTEURS_Test (void);
void MOTEURS_Avance(uint32_t distance);
void MOTEURS_Tourne(uint32_t tours);
void MOTEURS_Stop(void);

void MOTEURS_TimerEncodeurUpdate (TIM_HandleTypeDef *htim);

#endif /* INC_MOTEURS_H_ */
