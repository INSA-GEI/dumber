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
#include <stm32f10x.h>
#include <string.h>
#include <stdio.h>

#include "system_dumby.h"
#include "battery.h"
#include "led.h"
#include "motor.h"
#include "usart.h"

Settings Dumber;

/** @addtogroup Projects
 * @{
 */

/** @addtogroup system_dumby
 * @{
 */

/**
 *@brief	Configure les propriété de dumber au démarage :
 *			Etat de depart		: IDLE
 *			Liaison série		: USART
 *@param 	None
 *@retval 	None
 */
void systemConfigure(void)
{
    GPIO_InitTypeDef Init_Structure;

    // Configure le systick pour générer des interruptions toutes les 10ms.
    SysTick_Config(SystemCoreClock / 100); //configuration du systick à 10ms

    // Assigne et défini le GPIO necessaire pour la gestion du shutdown.
    Init_Structure.GPIO_Pin = GPIO_Pin_5;
    Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
    Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    GPIO_Init(GPIOB, &Init_Structure);

    // Initialise la structure system Dumber
    Dumber.BatteryPercentage = UNDEFINED;
    Dumber.BatteryCurrent = UNDEFINED;
    // Retourne le pourcentage de charge de la batterie
    Dumber.StateSystem = STATE_IDLE;								// État de la MAE
    Dumber.AddOn = FALSE;							// Un AddOn a été détecté
    Dumber.BatterieChecking = FALSE; // On doit vérifier la valeur de la batterie
    Dumber.WatchDogStartEnable = FALSE;	// Le Robot a été lancé en mode WithWatchDog ou WithoutWatchDog
    Dumber.InvalidWatchdogResetCpt=0;
    Dumber.cpt_watchdog = 0;
    Dumber.cpt_systick = 0;
    Dumber.cpt_inactivity = 0;
    Dumber.acquisition = FALSE;
    Dumber.busyState = FALSE;
    Dumber.stateBattery = 2;
}

/**
 *	@brief Désactive les interruptions et entre dans une boucle while (1) en attendant l'extinction du CPU.
 *	@param None
 */

void systemShutDown(void) {
    __disable_irq();
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    while (1);
}

/**
 * @brief Bascule le système dans un état.
 *
 * @param    state: Nouvel état
 * @retval   None
 */
void systemChangeState(States state) {
    switch (state)
    {
        case STATE_IDLE:
            Dumber.StateSystem = STATE_IDLE;
            Dumber.WatchDogStartEnable = FALSE;
            Dumber.cpt_watchdog = 0;
            Dumber.InvalidWatchdogResetCpt=0;
            Dumber.cpt_systick = 0;
            GPIO_SetBits(GPIOA,GPIO_Pin_12); // Désactive les encodeurs
            motorCmdLeft(BRAKE, 0);
            motorCmdRight(BRAKE, 0);
            break;

        case STATE_RUN:
            Dumber.StateSystem=STATE_RUN;
            GPIO_ResetBits(GPIOA,GPIO_Pin_12); // Active les encodeurs
            Dumber.cpt_watchdog=0;
            break;

        case STATE_LOW:
            Dumber.StateSystem=STATE_LOW;
            GPIO_ResetBits(GPIOA,GPIO_Pin_12); // Active les encodeurs
            Dumber.cpt_watchdog=0;
            break;

        case STATE_DISABLE:
            Dumber.StateSystem=STATE_DISABLE;
            GPIO_SetBits(GPIOA,GPIO_Pin_12); // Désactive les encodeurs
            motorCmdRight(BRAKE,0);
            motorCmdLeft(BRAKE,0);
            Dumber.WatchDogStartEnable = FALSE;

            cptMesureHigh=0;
            cptMesureLow=0;
            cptMesureDisable=0;
            Dumber.stateBattery= 0;
            break;

        case STATE_WATCHDOG_DISABLE:
            Dumber.StateSystem=STATE_WATCHDOG_DISABLE;
            GPIO_SetBits(GPIOA,GPIO_Pin_12); // Désactive les encodeurs
            motorCmdRight(BRAKE,0);
            motorCmdLeft(BRAKE,0);
            Dumber.WatchDogStartEnable = FALSE;
            break;

        default:
            /* Unknown state -> go into DISABLE */
            systemChangeState(STATE_DISABLE);
    }
}

/**
 * @brief   Remise à zéro du watchdog, en fonction de l'état en cours.
 *
 * @param   None
 * @retval  None
 */
char systemResetWatchdog(void) {
    char resultat =0;

    if ((Dumber.StateSystem == STATE_RUN) || (Dumber.StateSystem == STATE_LOW)) { // si on est actif
        if (Dumber.WatchDogStartEnable == TRUE) { // si le watchdog est lancé
            if ((Dumber.cpt_watchdog >= WATCHDOG_MIN) && (Dumber.cpt_watchdog <= WATCHDOG_MAX)) { // si le watchdog est dans sa plage réarmable

                Dumber.InvalidWatchdogResetCpt=0; // on remet le compteur d'erreur à zéro
            } else {
                Dumber.InvalidWatchdogResetCpt++; // on incrémente le compteur d'erreur
                if (Dumber.InvalidWatchdogResetCpt>WATCHDOG_INVALID_COUNTER_MAX) // on a atteint le max d'erreur possible
                    systemChangeState(STATE_WATCHDOG_DISABLE); // le système est désactivé
            }

            Dumber.cpt_watchdog = 0; // on remet le watchdog à zéro

            resultat=1;
        }
    }

    return resultat;
}

/** @addtogroup Gestion Gestions compteurs systick
 * @{
 */


/**
 * @brief Interruption systick. Gestion des timers internes.
 *		  Défini la vitesse de la loi d'asservissement.
 *		  Défini les valeurs de shutdown.
 *		  Défini les valeurs d'inactivités.
 *		  Défini la valeur de clignotement de la LED.
 *
 * @note  Tout ces temps sont calculés sous base du systick.
 */
void SysTick_Handler(void){
    Dumber.cpt_systick+=10;
    Dumber.cpt_inactivity+=10;

    if(asservissement == 1){
        regulation_vitesseD =1;
        regulation_vitesseG =1;
    }
    if(Dumber.cpt_systick % TIMER_1s==0) Dumber.cpt_systick=0;

    /* Gestion du watchdog */
    if((Dumber.WatchDogStartEnable == TRUE) && (Dumber.StateSystem != STATE_IDLE))
        Dumber.cpt_watchdog+=10;
    else
        Dumber.cpt_watchdog=0;

    if (Dumber.cpt_watchdog > WATCHDOG_MAX) {
        Dumber.cpt_watchdog=30; // pour avoir toujours un watchdog cadancé à 1000 ms, et pas 1030ms
        Dumber.InvalidWatchdogResetCpt++;
    }

    if (Dumber.InvalidWatchdogResetCpt > WATCHDOG_INVALID_COUNTER_MAX) {
        systemChangeState(STATE_WATCHDOG_DISABLE);
    }

    if(Dumber.cpt_systick % 500 == 0){
        Dumber.acquisition=VOLTAGE;
        batteryRefreshData();
    }

    /*if(Dumber.cpt_systick % 100==0){
        etatLED++;

        if (etatLED ==12) etatLED = 0;
    }*/

#if !defined (__NO_INACTIVITY_SHUTDOWN__)
    if(Dumber.cpt_inactivity>=120000){
        systemShutDown();
    }
#else
#warning "Shutdown after inactivity period disabled! Not for production !!!"
#endif /* __NO_INACTIVITY_SHUTDOWN__ */

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
