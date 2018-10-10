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
#include "system_dumby.h"
#include "led.h"

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
char etatLED;
char LEDON;

#define LED_GREEN   0
#define LED_RED     1
#define LED_ORANGE  2

/*void ledGreenOn(void);
void ledRedOn(void);
void ledOff(void);
void ledOrangeOn(void);*/

/**
 * @brief Configure le GPIO PB0 et PB1 afin de contrôler la led.
 * @param Aucun
 */
void ledConfigure(void)
{
    GPIO_InitTypeDef Init_Structure;

    // Configure les PIN B0 et B1 en output / alternate fonction
    Init_Structure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &Init_Structure);

    etatLED = 1;
    LEDON =0;
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
void ledOrangeOn(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    GPIO_SetBits(GPIOB,GPIO_Pin_0);
}

/**
 * @brief Allume une LED de couleur rouge.
 * @param Aucun
 */
void ledRedOn(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
    GPIO_SetBits(GPIOB,GPIO_Pin_0);
}

/**
 * @brief Allume une LED de couleur verte.
 * @param Aucun
 */
void ledGreenOn(void)
{

    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    GPIO_ResetBits(GPIOB,GPIO_Pin_0);
}

/**
 * @brief Eteint les LED.
 * @param Aucun
 */
void ledOff(void)
{
    GPIO_ResetBits(GPIOB,GPIO_Pin_0);
    GPIO_ResetBits(GPIOB,GPIO_Pin_1);
}

/**
 * @brief Allume une couleur de LED.
 * @param Aucun
 */
void ledOn(char color)
{
    switch (color)
    {
        case LED_GREEN:
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
            GPIO_ResetBits(GPIOB,GPIO_Pin_0);
            break;

        case LED_RED:
            GPIO_ResetBits(GPIOB, GPIO_Pin_1);
            GPIO_SetBits(GPIOB,GPIO_Pin_0);
            break;

        case LED_ORANGE:
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
            GPIO_SetBits(GPIOB,GPIO_Pin_0);
            break;

        default:
            ledOff();
    }
}

/**
 * @brief Gere l'etat du clignotement de la led en fonction de l'etat du système
 *        Appelée toutes les 10 ms
 * @param state: état actuel du système
 * @param batteryState: état de la batterie
 */
void ledManagement(States state, char batteryState) {
    static char ledCounter=0;
    char color;

    if (batteryState>1) color=LED_GREEN;
    else color = LED_RED;

    switch (state)
    {
        case STATE_IDLE:
            if ((ledCounter<15) || ((ledCounter>=100) && (ledCounter<115))) ledOn(color);
            else ledOff();
            break;

        case STATE_RUN:
        case STATE_LOW:
            ledOn(color);
            break;

        case STATE_WATCHDOG_DISABLE:
            if (ledCounter<100) ledOn(LED_RED);
            else ledOff();
            break;

        case STATE_DISABLE:
        default:
            if (ledCounter%10 ==0) ledOn(LED_RED);
            else ledOff();
    }

    ledCounter++;
    if (ledCounter>200) ledCounter=0;
}

//void ledManagement(States state) {
//    if (state == STATE_IDLE) {
//        if (etatLED == 1) {
//            LEDON = 1;
//        } else if (etatLED == 2)
//            LEDON = 0;
//    }
//
//    if (state == STATE_RUN || state == STATE_LOW)
//        LEDON = 1;
//
//    if (state == STATE_DISABLE) {
//        if (etatLED % 2 == 0) ledRedOn();
//        else ledOff();
//    }
//    else if (LEDON) {
//        if (Dumber.stateBattery == 1 && Dumber.StateSystem != STATE_DISABLE)
//            ledOrangeOn();
//        else if (Dumber.stateBattery == 2 && Dumber.StateSystem != STATE_DISABLE)
//            ledGreenOn();
//    } else
//        ledOff();
//}
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
