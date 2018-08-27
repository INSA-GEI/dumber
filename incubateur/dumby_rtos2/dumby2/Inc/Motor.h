#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f1xx_hal.h"


extern uint16_t distanceR;
extern uint16_t distanceL;

extern float speedR;
extern float speedL;

void MOTOR_LEFT_SET(uint8_t state, uint16_t speed);
void MOTOR_RIGHT_SET(uint8_t state, uint16_t speed);
void MOTORS_SET(uint8_t state, uint16_t speed);
void MOTOR_INIT(uint8_t motor);
void ENCODER_INIT(void);
uint8_t borneVitesse(int vitesse);
unsigned int valabs(int val);

void TIM1_Update_Callback(void);
#endif

