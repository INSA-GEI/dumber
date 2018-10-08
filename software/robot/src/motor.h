/**
 ******************************************************************************
 * @file    motor.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    19-June-2017
 * @brief   Gestion des moteurs du robots
 *
 *			Ce fichier contient les fonctions lié à la gestions du moteur :
 *			- L'initialisation des PIN ultisé par les moteurs et encodeurs
 * 			- L'initialisation du Timer2 servant de bases de temps aux PWM pour
 * 			  moteur1 et moteur2
 *			- L'initialisation du Timer3 servant à échantilloner les capteur magnétique de position
 *			  des 2 moteurs.
 * 			- La commandes des moteurs dans leurs 3 modes de fonctionnement (AVANT, ARRIERE, FREIN)
 * 			  et de leurs vitesse (valeur de PWM entre 0 et 255).
 *
 *@note			TABLE DE VERITEE DU DRIVER MOTEUR
 *
 *	ENABLE 			        |			INPUTS		| 		 Moteurs
 *					|					|
 *		0			|				X	| Roue Libre
 *--------------------------------------------------------------------
 *					|  CMDA & !CMDB 			| Avant
 *		1			|  !CMDA & CMDB				| Arriére
 *					|  CMDA = CMDB				| Frein
 ******************************************************************************
 ******************************************************************************
 */
#ifndef Motor_H
#define Motor_H

#include "stm32f10x.h"
//#include "system_dumby.h"

extern uint16_t G_speedRight, G_speedLeft, G_lapsLeft, G_lapsRight;
extern uint16_t tourG, tourD, tourPositionD,tourPositionG;

/**
 * @brief Etats que peuvent prendre chaques moteurs
 * 		  Avant, Arriére, Frein
 */
#define FORWARD 11  
#define REVERSE 12 
#define BRAKE 	13 

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
//void regulationCBK(void);

#endif /* Motor_H */

