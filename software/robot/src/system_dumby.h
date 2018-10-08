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
#ifndef system_dumby_H
#define system_dumby_H

#include "stm32f10x.h"

/* Declaration des Constantes */
#define VERSION				"version 1.2\r"

#define SPI 				10 
#define USART 				20 
#define I2C				30 

#define TRUE  				40 
#define FALSE 				50 

#define RUN				51 
#define	IDLE				52
#define LOW				53 
#define DISABLE 			54

#define	VOLTAGE 			98
#define CURRENT 			99

#define	UNDEFINED			101 

//CMDE

#define	PingCMD				'p'		
#define ResetCMD			'r'			
#define SetMotorCMD			'm'			
#define StartWWatchDogCMD		'W'			
#define	ResetWatchdogCMD		'w'				
#define GetBatteryVoltageCMD		'v'			
#define GetVersionCMD			'V'			
#define StartWithoutWatchCMD 		'u'
#define MoveCMD				'M'			
#define TurnCMD				'T'
#define BusyStateCMD 			'b'	
#define TestCMD                 	't'
#define DebugCMD			'a'

#define SystemCoreClock 		8000000

//extern volatile uint16_t voltageADC;

#define TIMER_1s 			1000 // 1 sec
#define TIMER_Watchdog			3050	 //
#define TTMER_Inactivity 		120000 // 2 min

#define RIGHT				'>'
#define LEFT				'<'

#define OK_ANS				"O\r"
#define ERR_ANS				"E\r"
#define UNKNOW_ANS			"C\r"
#define BAT_OK				"2\r"
#define BAT_LOW				"1\r"
#define BAT_EMPTY			"0\r"

#define COMMONSPEED			5
#define LOWSPEED			2
#define HYPERVITESSE			7

/* Declaration de structure */

typedef struct Settings Settings;
struct Settings
{	
        // Information
        uint16_t BatteryPercentage;			// Retourne le pourcentage de charge de la batterie
        uint16_t BatteryCurrent;				//
        char StateSystem;								// Etat de la MAE
        char AddOn;											// Un AddOn a �t� d�tect�							// Les instructions seront sur le protocol SPI
        char BatterieChecking; 					// On doit verifier la valeur de la batterie
        uint16_t cpt_systick;
        char WatchDogStartEnable;				// Le Robot a �t� lanc� en mode WithWatchDog ou WithoutWatchDog
        uint16_t cpt_watchdog;
        char busyState;
        int  cpt_inactivity;
        char acquisition;
        char stateBattery;
        char flagSystick;
};

/* Declaration des variables systemes */
extern uint16_t greenLight;
extern uint16_t redLight;
extern Settings Dumber;
extern uint16_t asservissement;
extern uint16_t regulation_vitesseD, regulation_vitesseG;
extern uint16_t watchDogState;
extern char etatLED; // Tout les 200 ms cette variable s'incr�mente de 1 jusqu'� 5
extern char vClignotement1s;
extern char LEDON;

/* Prototype Fonctions */
void default_settings(void);
void MAP_PinShutDown(void);
void SysTick_Handler(void);
void Configure_SysTick(void);
void shutDown(void);

#endif /* SYSTEM_DUMBY_H_ */


