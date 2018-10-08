/**
 ******************************************************************************
 * @file    main.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    16-mai-2016
 * @brief   Programme principale du robot Dumby.
 *			Dumby est un robot utilis� par l'INSA toulouse lors des TPs de temps
 *			R�el en 4�me ann�e. Ce fichier est le fichier principal et la
 *			derni�re version de son software. Le micro-controleur utilis� est
 *			un STM32-103-RB.
 *			Il comporte comme fonctionnalit�e entre autre :
 *			- Asservisement des moteurs
 *			- Detection de tension de batterie
 *			- Gestion de commande via l'uart
 *			La tache du fichier est de :
 *			- Alimenter les horloges des periph�riques necessaires.
 *			- Appeller les sous programme necessaire au bon fonctionnement
 *			  du robot.
 ******************************************************************************
 */
#include <battery.h>
#include <stm32f10x.h>
#include <string.h>
#include <stdio.h>

#include "system_dumby.h"
#include "cmde_usart.h"
#include "motor.h"
#include "led.h"

#include "debug.h"

float integration1 = 0;
float integration2 = 0;
/**
 * @brief  Coefficient PI
 */
const float kp = 15;
/**
 * @brief  Coefficient PI
 */
const float ki = 1.5;
/**
 * @brief  Valeurs de vitesse
 */
float motD = 0, motG = 0;
/**
 * @brief  erreurs entre vitesse reel et moteurs (droite)
 */
int erreurD;
/**
 * @brief  erreurs entre vitesse reel et moteurs (gauche)
 */
int erreurG;

char cptMesureHigh=0;
char cptMesureLow=0;
char cptMesureDisable=0;
uint32_t cptMesureEmergencyHalt=0;

uint16_t vbatLowerVal;
uint16_t vbatHighVal;
uint16_t vbatDiff;

uint16_t testPostion=0;
uint32_t mesureVoltage;
uint32_t meanVoltage;

#define COMPTEUR_SEUIL_HIGH		8
#define COMPTEUR_SEUIL_LOW		8
#define COMPTEUR_SEUIL_DISABLE		8
#define COMPTEUR_SEUIL_EMERGENCY_HALT	3000

void Configure_Clock_Periph(void);

/**
 * @brief  Initialise les horloges du micro et de ses p�riph�riques.
 * @param  None
 * @retval None
 */
int main(void)
{	
    uint16_t k;
    /**
     * Initialisation
     */

    Configure_Clock_Periph();
    default_settings();

    MAP_PinShutDown();

    MAP_MotorPin();
    MAP_LEDpin();
    MAP_UsartPin();
    MAP_batteryPin();
    INIT_TIM2();
    INIT_OCMotorPwm();
    Configure_SysTick();

    INIT_USART();
    INIT_IT_UsartReceive();

    DMA_BAT();
    ADC1_CONFIG();
    INIT_IT_DMA();
    IC_TIM1_CHANEL3();
    IC_TIM1_CHANEL1();
    IT_TIM1();
    GPIO_ResetBits(GPIOA,GPIO_Pin_12); //enable encodeurs

    while (1){
        __WFE(); // Bascule la puce en sleep mode

        if (Dumber.flagSystick == 1)
        {
            Dumber.flagSystick = 0;

            if(Dumber.acquisition==VOLTAGE && Dumber.BatterieChecking==TRUE)
            {
                vbatLowerVal = 0xFFF;
                vbatHighVal = 0;

                for(k=0; k<VOLTAGE_BUFFER_SIZE; k++)
                {
                    meanVoltage+=ADCConvertedValue[k];

                    if (vbatLowerVal> ADCConvertedValue[k]) vbatLowerVal = ADCConvertedValue[k];
                    if (vbatHighVal< ADCConvertedValue[k]) vbatHighVal = ADCConvertedValue[k];
                }

                vbatDiff = vbatHighVal - vbatLowerVal;

                meanVoltage= meanVoltage/VOLTAGE_BUFFER_SIZE;

                mesureVoltage = meanVoltage;

                Dumber.BatteryPercentage = mesureVoltage;
                Dumber.acquisition=FALSE;

                if(Dumber.BatteryPercentage >= VBAT_SEUIL_LOW)
                {
                    cptMesureHigh++;
                    if(cptMesureHigh >= COMPTEUR_SEUIL_HIGH)
                    {
                        if(Dumber.StateSystem == LOW)	Dumber.StateSystem = RUN;

                        Dumber.stateBattery = 2;
                        cptMesureHigh=0;
                        cptMesureLow=0;
                        cptMesureDisable=0;
                        cptMesureEmergencyHalt=0;
                    }
                }
                else if (Dumber.BatteryPercentage < VBAT_SEUIL_LOW && Dumber.BatteryPercentage >= VBAT_SEUIL_DISABLE)
                {
                    cptMesureLow++;
                    if(cptMesureLow >= COMPTEUR_SEUIL_LOW)
                    {
                        if(Dumber.StateSystem == RUN)	Dumber.StateSystem=LOW;

                        Dumber.stateBattery =1;
                        cptMesureHigh=0;
                        cptMesureLow=0;
                        cptMesureDisable=0;
                    }
                }
                else // Dumber.BatteryPercentage < VBAT_SEUIL_DISABLE
                {
                    cptMesureDisable++;

                    if(cptMesureDisable >= COMPTEUR_SEUIL_DISABLE)
                    {
                        Dumber.StateSystem = DISABLE;
                        cptMesureHigh=0;
                        cptMesureLow=0;
                        cptMesureDisable=0;
                        Dumber.stateBattery= 0;

                        cmdRightMotor(BRAKE,0);
                        cmdLeftMotor(BRAKE,0);
                    }
                }
            }

            if (regulation_vitesseD) {
                erreurD = (signed int) G_speedRight - (signed int) tourD;
                motD = kp * erreurD + integration1;
                integration1 += ki * erreurD;

                if (motD > 255)	motD = 255;

                if (motD < 0) motD = 0;

                motD = (uint16_t) motD;
                majVitesseMotorD(motD);
                tourD = 0;
                regulation_vitesseD = 0;

                if (G_lapsRight - tourPositionD < 0) {
                    cmdRightMotor(BRAKE, 255);
                }
            }

            if (regulation_vitesseG) {
                erreurG = (signed int) G_speedLeft - (signed int) tourG;
                motG = kp * erreurG + integration2;

                integration2 += ki * erreurG;

                if (motG > 255)	motG = 255;

                if (motG < 0) motG = 0;

                motG = (uint16_t) motG;

                majVitesseMotorG(motG);
                tourG = 0;
                regulation_vitesseG = 0;

                if (G_lapsLeft - tourPositionG < 0) {
                    cmdLeftMotor(BRAKE, 255);
                }
            }

            if (G_lapsLeft - tourPositionG < 0 && G_lapsRight - tourPositionD < 0
                    && asservissement == 1) {

                cmdLeftMotor(BRAKE, 255);
                cmdRightMotor(BRAKE, 255);
                asservissement = 0;
                erreurD = 0;
                erreurG = 0;
                integration1 = 0;
                integration2 = 0;
                Dumber.busyState = FALSE;
                Dumber.cpt_inactivity = 0;
            }

            if (Dumber.StateSystem == IDLE) {
                if (etatLED == 1) {
                    LEDON = 1;
                } else if (etatLED == 2)
                    LEDON = 0;
            }

            if (Dumber.StateSystem == DISABLE) {
                if (etatLED % 2 == 0) LEDred();
                else LEDoff();

                cptMesureEmergencyHalt++;

                if (cptMesureEmergencyHalt >= COMPTEUR_SEUIL_EMERGENCY_HALT)
                {
                    shutDown();
                    while (1);
                }
            }

            if (Dumber.StateSystem == RUN || Dumber.StateSystem == LOW)
                LEDON = 1;

            if (LEDON) {
                if (Dumber.stateBattery == 1 && Dumber.StateSystem != DISABLE)
                    LEDorange();
                if (Dumber.stateBattery == 2 && Dumber.StateSystem != DISABLE)
                    LEDgreen();
            } else
                LEDoff();
        }
    }

#ifndef __GNUC__
#pragma diag_suppress 111
#endif /* __GNUC__ */
    return 0;
}

/**
 * @brief  Initialise les horloges du micro et de ses p�riph�riques.
 * @param  None
 * @retval None
 */
void Configure_Clock_Periph(void)
{
    //Configuration de la fr�qyence d'horloge de l'adc */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    //Activation de l'horloge du GPIO, de A B et C, de ADC1, de AFIO
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
