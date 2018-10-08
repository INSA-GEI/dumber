/**
 ******************************************************************************
 * @file    led.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    16-mai-2016
 * @brief   Gestion des clignotements des LED
 *			Ce fichier fournis des fonctions afin de faciliter
 *			la gestions des changements de couleurs.
 *
 ******************************************************************************
 ******************************************************************************
 */

#include <stm32f10x.h>
#include "led.h"

#include "system_dumby.h"

/** @addtogroup Projects
 * @{
 */

/** @addtogroup Led_color
 * @{
 */



/** @addtogroup Init Init GPIO
 * @{
 */

TIM_TimeBaseInitTypeDef TIM_TimeBaseLED;
TIM_OCInitTypeDef TIM_OCConfigure;

/**
 * @brief Configure le GPIO PB0 et PB1 afin de contr�ler la led.
 * @param Aucun
 */
void MAP_LEDpin(void)
{
    GPIO_InitTypeDef Init_Structure;

    // Configure les PIN B0 et B1 en output / alternate fonction
    Init_Structure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &Init_Structure);
}

/**
 * @}
 */

/** @addtogroup change Change color
 * @{
 */
/**
 * @brief Allume une LED de couleur orange.
 * @param Aucun
 */
void LEDorange(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    GPIO_SetBits(GPIOB,GPIO_Pin_0);
}

/**
 * @brief Allume une LED de couleur rouge.
 * @param Aucun
 */
void LEDred(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
    GPIO_SetBits(GPIOB,GPIO_Pin_0);
}

/**
 * @brief Allume une LED de couleur verte.
 * @param Aucun
 */
void LEDgreen(void)
{

    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    GPIO_ResetBits(GPIOB,GPIO_Pin_0);
}

/**
 * @brief Eteint la LED.
 * @param Aucun
 */
void LEDoff(void)
{
    GPIO_ResetBits(GPIOB,GPIO_Pin_0);
    GPIO_ResetBits(GPIOB,GPIO_Pin_1);
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
