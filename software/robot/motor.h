#ifndef Motor_H
#define Motor_H

#include "stm32f10x.h"
#include "system_dumby.h"


extern uint16_t G_speedRight, G_speedLeft, G_lapsLeft, G_lapsRight;
extern uint16_t tourG, tourD, tourPositionD,tourPositionG;
/**
	* @brief Mode de fonctionnement des moteurs
	*/
#define FORWARD 11 // 
#define REVERSE 12 //
#define BRAKE 13 //

void MAP_MotorPin(void);
void INIT_TIM2(void);
void INIT_OCMotorPwm(void);
void commandeMoteur(void);

void IC_TIM1_CHANEL3(void);
void IC_TIM1_CHANEL1(void);
void IT_TIM1(void);

void cmdRightMotor(char mod, uint16_t pwm);
void cmdLeftMotor(char mod, uint16_t pwm);

void majVitesseMotorG(uint16_t pwm);
void majVitesseMotorD(uint16_t pwm);

void regulationMoteur(char modRight, char modLeft, uint16_t lapsRight, uint16_t lapsLeft, uint16_t speedRight, uint16_t speedLeft);


#endif /* Motor_H */

