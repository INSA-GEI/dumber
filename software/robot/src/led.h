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
#ifndef LED_H
#define LED_H

#include "stm32f10x.h"

#define RED	255
#define GREEN	250

void MAP_LEDpin(void);
void INIT_TIM3Led(void);
void LEDgreen(void);
void LEDred(void);
void LEDoff(void);
void LEDorange(void);

#endif /* LED_H */
