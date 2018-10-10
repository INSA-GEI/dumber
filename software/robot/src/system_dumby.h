/**
 ******************************************************************************
 * @file    system_dumby.h
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    19-June-2017
 * @brief	Configure les variables globals, les timers necessaires.
 *
 *			Configure la pin de shutdown.
 *			Initialise le systick.
 *			Initialise les valeurs de shutdown avec le watchdog.
 *			Initialise les variables global des commandes.
 *			Initialise la strucutre Settings contenant les paramètres de dumber.
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
#ifndef _SYSTEM_DUMBY_H_
#define _SYSTEM_DUMBY_H_

#include "stm32f10x.h"

/* Déclaration des Constantes */
#define VERSION				"version 1.4\r"

#define SPI 				10 
#define USART 				20 
#define I2C				    30

#define TRUE  				40 
#define FALSE 				50 

#define	VOLTAGE 			98
#define CURRENT 			99

#define	UNDEFINED			101 

#define TIMER_1s 			1000    // 1 sec
//#define TIMER_Watchdog		3050
//#define TTMER_Inactivity 	120000  // 2 min

#define WATCHDOG_MIN        970
#define WATCHDOG_MAX        1030
#define WATCHDOG_INVALID_COUNTER_MAX 3

#define COMMONSPEED			5
#define LOWSPEED			2
#define HYPERVITESSE		7

/* Déclaration de structure */

enum States {
        STATE_IDLE = 0,
        STATE_RUN,
        STATE_LOW,
        STATE_DISABLE,
        STATE_WATCHDOG_DISABLE
};
typedef enum States States;

struct Settings
{	
        // Information
        uint16_t BatteryPercentage;			// Retourne le pourcentage de charge de la batterie
        uint16_t BatteryCurrent;			//
        States StateSystem;					// État de la MAE
        char AddOn;							// Un AddOn a été détecté
        // Les instructions seront sur le protocole SPI
        char BatterieChecking; 				// On doit vérifier la valeur de la batterie
        uint16_t cpt_systick;
        char WatchDogStartEnable;			// Le Robot a été lancé en mode WithWatchDog ou WithoutWatchDog
        uint16_t cpt_watchdog;
        uint8_t InvalidWatchdogResetCpt;     // Compteur de remise à zéro du watchdog en dehors du temps imparti
        char busyState;
        int  cpt_inactivity;
        char acquisition;
        char stateBattery;
        char flagSystick;
};

typedef struct Settings Settings;

/* Déclaration des variables systèmes */
extern Settings Dumber;

/* Prototype Fonctions */
void systemConfigure(void);
void systemChangeState(States state);
char systemResetWatchdog(void);
void systemShutDown(void);

#endif /* _SYSTEM_DUMBY_H_ */


