#ifndef system_dumby_H
#define system_dumby_H

#include "stm32f10x.h"

/* Declaration des Constantes */
#define VERSION				"version 0.2\r"

#define SPI 	10 //
#define USART 20 //¶
#define I2C		30 //

#define TRUE  40 // (
#define FALSE 50 // 2

#define RUN		51 // 3
#define	IDLE	52 //4
#define LOW		53 // 5
#define DISABLE 54

#define	VOLTAGE 98
#define CURRENT 99

#define	UNDEFINED	101 

//CMDE

#define	PingCMD											'p'		
#define ResetCMD										'r'			
#define SetMotorCMD									'm'			
#define StartWWatchDogCMD						'W'			
#define	ResetWatchdogCMD						'w'				
#define GetBatteryVoltageCMD				'v'			
#define GetVersionCMD								'V'			
#define StartWithoutWatchCMD 				'u'
#define MoveCMD											'M'			
#define TurnCMD											'T'
#define BusyStateCMD 								'b'	

#define SystemCoreClock 8000000

/**
	*	Base de temps utilisé (en ms)
	*	@TimeWatchDog 		Temps d'incrémentation d'un compteur watchdog
	*	@WDCptReset 			Nombre de fois que le compteur doit s'incrémenter avant 
	*	@TimeBeforeReset	Temps avant le reset total de l'appareil
	*/

#define TIMER_1s 		1000 // 1 sec
#define TIMER_Watchdog				3050	 //
#define TTMER_Inactivity 120000 // 2 min


#define RIGHT												'>'
#define LEFT												'<'

#define OK_ANS											"O\r"
#define ERR_ANS											"E\r"
#define UNKNOW_ANS									"C\r"
#define BAT_OK											"2\r"
#define BAT_LOW											"1\r"
#define BAT_EMPTY										"0\r"

#define COMMONSPEED									17
#define LOWSPEED										11
#define HYPERVITESSE								100

/* Declaration de structure */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////																		SETTINGS									  															 /////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Settings Settings;
struct Settings
{	
	// Information
	uint16_t BatteryPurcentage;			// Retourne le pourcentage de charge de la batterie
	uint16_t BatteryCurrent;				//
	char StateSystem;								// Etat de la MAE
	char AddOn;											// Un AddOn a été détecté							// Les instructions seront sur le protocol SPI
	char BatterieChecking; 					// On doit verifier la valeur de la batterie  
	uint16_t cpt_systick;
	char WatchDogStartEnable;				// Le Robot a été lancé en mode WithWatchDog ou WithoutWatchDog
	uint16_t cpt_watchdog;
	char busyState;
	int	cpt_inactivity;
	char acquisition;
	char stateBattery;
};


/* Declaration des variables systemes*/
extern uint16_t greenLight;
extern uint16_t redLight;
extern Settings Dumber;
extern uint16_t asservissement;
extern uint16_t regulation_vitesseD, regulation_vitesseG;
extern uint16_t watchDogState;
extern char etatLED; // Tout les 200 ms cette variable s'incrémente de 1 jusqu'à 5
extern char vClignotement1s;
extern char LEDON;

/* Prototype Fonctions */
void default_settings(void);
void MAP_PinShutDown(void);
void SysTick_Handler(void);
void Configure_SysTick(void);
void shutDown(void);


#endif


