/**
 ******************************************************************************
 * @file    system_dumby.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    19-June-2017
 * @brief	Configure les variables globals, les timers necessaires.
 *
 *			Configure la pin de shutdown.
 *			Initialise le systick.
 *			Initialise les valeurs de shutdown avec le watchdog.
 *			Initialise les variables global des commandes.
 *			Initialise la strucutre Settings contenant les paramétres de dumber.
 *
 *@note		Les différentes commande sont :
 *			PingCMD							'p'
 *			ResetCMD 						'r'
 *			SetMotorCMD 					'm'
 *			StartWWatchDogCMD				'W'
 *			ResetWatchdogCMD				'w'
 *			GetBatteryVoltageCMD			'v'
 *			GetVersionCMD					'V'
 *			StartWithoutWatchCMD 			'u'
 *			MoveCMD							'M'
 *			TurnCMD							'T'
 *			BusyStateCMD 					'b'
 ******************************************************************************
 ******************************************************************************
 */
#include <battery.h>
#include "system_dumby.h"
#include <stm32f10x.h>
#include "led.h"
#include <string.h>
#include <stdio.h>
#include "motor.h"
#include "cmde_usart.h"

Settings Dumber;
static __IO uint32_t TimingBattery=10000;
uint16_t greenLight = 0;
uint16_t redLight = 0;
uint16_t regulation_vitesseD = 0, regulation_vitesseG = 0, asservissement =0;
char etatLED = 1;
uint16_t watchDogState;;
char vClignotement1s=0;
uint16_t tourPositionGprec =1, tourPositionDprec=1;
char LEDON =0;

int led=0;

/** @addtogroup Projects
 * @{
 */

/** @addtogroup system_dumby
 * @{
 */



/** @addtogroup init shutdown_gpio init
 * @{
 */

/**
 * @brief 		Assigne et défini le GPIO necessaire pour la gestion du shutdown.
 *
 * @param  		None
 * @retval 		None
 */
void MAP_PinShutDown(void)
{
    GPIO_InitTypeDef Init_Structure;
    Init_Structure.GPIO_Pin = GPIO_Pin_5;
    Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
    Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    GPIO_Init(GPIOB, &Init_Structure);
}

/**
 * @}
 */

/**
 *@brief	Configure les propriété de dumber au démarage :
 *			Etat de depart		: IDLE
 *			Liaison série		: USART
 *@param 	None
 *@retval 	None
 */
void default_settings(void)
{
    Dumber.BatteryPercentage = UNDEFINED;
    Dumber.BatteryCurrent = UNDEFINED;
    // Retourne le pourcentage de charge de la batterie
    Dumber.StateSystem = IDLE;								// Etat de la MAE
    Dumber.AddOn = FALSE;							// Un AddOn a été détecté
    Dumber.BatterieChecking = FALSE; // On doit verifier la valeur de la batterie
    Dumber.WatchDogStartEnable = FALSE;	// Le Robot a été lancé en mode WithWatchDog ou WithoutWatchDog
    Dumber.cpt_watchdog = 0;
    Dumber.cpt_systick = 0;
    Dumber.cpt_inactivity = 0;
    Dumber.acquisition = FALSE;
    Dumber.busyState = FALSE;
    Dumber.stateBattery = 2;
}

/**
 *@brief	Configure le systick pour générer des interruptions toutes les 50ms.
 *
 *@param 	None
 *@retval 	None
 */
void Configure_SysTick(void) {
    SysTick_Config(SystemCoreClock / 100); //configuration du systick à 50ms
}

/**
 *	@brief Désactive les interruptions et entre dans une boucle while (1) en attendant l'extinction du CPU.
 *	@param None
 */

void shutDown(void) {
    __disable_irq();
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    while (1);
}


/** @addtogroup Gestion Gestions compteurs systick
 * @{
 */


/**
 * @brief Interruption systick. Gestion des timers internes.
 *		  Definis la vitesse de la loi d'asservissement.
 *		  Definis les valeurs de shutdown.
 *		  Definis les valeurs d'inactivit�s.
 *		  Definis la valeur de clignotement de la LED.
 *
 * @note  Tout ces temps sont calcul� sous base du systick.
 */
void SysTick_Handler(void){
    Dumber.cpt_systick+=10;
    Dumber.cpt_inactivity+=10;

    if((Dumber.WatchDogStartEnable == TRUE) && (Dumber.StateSystem != IDLE)) Dumber.cpt_watchdog+=10;


    if(asservissement == 1){
        regulation_vitesseD =1;
        regulation_vitesseG =1;
    }
    if(Dumber.cpt_systick % TIMER_1s==0) Dumber.cpt_systick=0;

    if((Dumber.cpt_watchdog % 1000 ) >=975 || (Dumber.cpt_watchdog % 1000) <=25)
        watchDogState=TRUE;
    else watchDogState=FALSE;

    if(Dumber.cpt_systick % 500 == 0){
        Dumber.acquisition=VOLTAGE;
        voltagePrepare();
    }

    if(Dumber.cpt_systick % 100==0){
        etatLED++;

        if(etatLED ==12) etatLED = 0;
    }

#if !defined (__NO_INACTIVITY_SHUTDOWN__)
    if(Dumber.cpt_inactivity>=120000){
        shutDown();
    }
#else
#warning "Shutdown after inactivity period disabled! Not for production !!!"
#endif /* __NO_INACTIVITY_SHUTDOWN__ */

    if(Dumber.cpt_watchdog>=TIMER_Watchdog)
    {
        Dumber.StateSystem=DISABLE;
        cmdRightMotor(BRAKE,0);
        cmdLeftMotor(BRAKE,0);
    }

    Dumber.flagSystick=1;
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
