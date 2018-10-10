/**
 ******************************************************************************
 * @file    led.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    19-June-2017
 * @brief   Gestion des clignotements des LED
 *
 *			Ce fichier fournis des fonctions afin de faciliter
 *			la gestions des changements de couleurs.
 *
 ******************************************************************************
 ******************************************************************************
 */
#ifndef _LED_H_
#define _LED_H_

#include "stm32f10x.h"

#define RED	    255
#define GREEN	250

extern char etatLED; // Tout les 200 ms cette variable s'incrémente de 1 jusqu'à 5
extern char LEDON;

void ledConfigure(void);
void ledManagement(States state, char batteryState);

#endif /* _LED_H_ */
