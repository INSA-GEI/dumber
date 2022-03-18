/**
 ******************************************************************************
 * @file    main.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    16-mai-2016
 * @brief   Programme principale du robot Dumby.
 *			Dumby est un robot utilisé par l'INSA toulouse lors des TPs de temps
 *			Réel en 4éme année. Ce fichier est le fichier principal et la
 *			dernière version de son software. Le micro-controleur utilisé est
 *			un STM32-103-RB.
 *			Il comporte comme fonctionnalitée entre autre :
 *			- Asservisement des moteurs
 *			- Detection de tension de batterie
 *			- Gestion de commande via l'uart
 *			La tache du fichier est de :
 *			- Alimenter les horloges des periphériques necessaires.
 *			- Appeller les sous programme necessaire au bon fonctionnement
 *			  du robot.
 ******************************************************************************
 */

#include <stm32f10x.h>
#include <string.h>
#include <stdio.h>

#include "system_dumby.h"
#include "motor.h"
#include "led.h"
#include "usart.h"
#include "battery.h"
#include "debug.h"

void mainConfigureClock(void);
void mainPeripheralsInit(void);

/**
 * @brief  Initialise les horloges du micro et de ses périphériques.
 * @param  None
 * @retval None
 */
int main(void)
{	
    /**
     * Initialisation
     */
    mainPeripheralsInit();

    while (1){
        __WFE(); // Bascule la puce en sleep mode

        if (Dumber.flagSystick == 1)
        {
            Dumber.flagSystick = 0;

            /* Gestion des niveaux de batterie */
            batteryManagement();

            /* Gestion des moteurs (asservissement, .. */
            motorManagement();

            /* Gestion du clignotement de la led, f=100Hz*/
            ledManagement(Dumber.StateSystem, Dumber.stateBattery);

            if (Dumber.StateSystem == STATE_DISABLE) {
                cptMesureEmergencyHalt++;

                if (cptMesureEmergencyHalt >= COMPTEUR_SEUIL_EMERGENCY_HALT) {
                    systemShutDown();
                    while (1);
                }
            }
        }
    }

#ifndef __GNUC__
#pragma diag_suppress 111
#endif /* __GNUC__ */
    return 0;
}

/**
 * @brief  Initialise les périphériques pour leur utilisation.
 * @param  None
 * @retval None
 */
void mainPeripheralsInit(void) {
    mainConfigureClock();

    systemConfigure();
    motorConfigure();
    ledConfigure();
    usartConfigure();
    batteryConfigure();
}

/**
 * @brief  Initialise les horloges du micro et de ses périphériques.
 * @param  None
 * @retval None
 */
void mainConfigureClock(void)
{
    //Configuration de la fréquence d'horloge de l'adc */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    //Activation de l'horloge du GPIO, de A, B et C, de ADC1, de AFIO
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM2|RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1| RCC_APB2Periph_TIM1 |RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
            RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO|RCC_APB2Periph_USART1|RCC_APB2Periph_SPI1,
            ENABLE);
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/*
 * Minimal __assert_func used by the assert() macro
 * */
void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
    while(1)
    {}
}

/*
 * Minimal __assert() uses __assert__func()
 * */
void __assert(const char *file, int line, const char *failedexpr)
{
    __assert_func (file, line, NULL, failedexpr);
}

#ifdef USE_SEE
#ifndef USE_DEFAULT_TIMEOUT_CALLBACK
/**
 * @brief  Basic management of the timeout situation.
 * @param  None.
 * @retval sEE_FAIL.
 */
uint32_t sEE_TIMEOUT_UserCallback(void)
{
    /* Return with error code */
    return sEE_FAIL;
}
#endif
#endif /* USE_SEE */
