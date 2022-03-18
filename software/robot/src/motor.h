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
 *		1			|  !CMDA & CMDB				| Arrière
 *					|  CMDA = CMDB				| Frein
 ******************************************************************************
 ******************************************************************************
 */
#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stm32f10x.h"

extern uint16_t G_speedRight, G_speedLeft, G_lapsLeft, G_lapsRight;
extern uint16_t tourG, tourD, tourPositionD,tourPositionG;

extern uint16_t asservissement;
extern uint16_t regulation_vitesseD, regulation_vitesseG;
/**
 * @brief États que peuvent prendre chaque moteurs
 * 		  Avant, Arrière, Frein
 */
#define FORWARD 11  
#define REVERSE 12 
#define BRAKE 	13 

void motorConfigure(void);
void motorManagement(void);

void motorCmdRight(char mod, uint16_t pwm);
void motorCmdLeft(char mod, uint16_t pwm);
void motorSpeedUpdateLeft(uint16_t pwm);
void motorSpeedUpdateRight(uint16_t pwm);
void motorRegulation(char modRight, char modLeft, uint16_t lapsRight, uint16_t lapsLeft, uint16_t speedRight, uint16_t speedLeft);

#endif /* _MOTOR_H_ */

