#include "system_dumby.h"
#include "Battery.h"
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

/**
	*	@brief configure les PIN de shutdown de dumber sur PB5 en OUTPP
	*	@param aucun
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
	*	@brief	Configure les propriété de dumber au démarage :
	*					Etat de depart	: IDLE
	*					Pas d'AddOn			:
	*					Liaison série		:	USART					
	*/
void default_settings(void)
{
	Dumber.BatteryPurcentage = UNDEFINED;
	Dumber.BatteryCurrent		 = UNDEFINED;
	// Retourne le pourcentage de charge de la batterie
	Dumber.StateSystem			 		= IDLE;								// Etat de la MAE
	Dumber.AddOn						 		= FALSE;							// Un AddOn a été détecté						
	Dumber.BatterieChecking  		= FALSE; 							// On doit verifier la valeur de la batterie  
	Dumber.WatchDogStartEnable 	= FALSE;							// Le Robot a été lancé en mode WithWatchDog ou WithoutWatchDog
	Dumber.cpt_watchdog 				= 0;
	Dumber.cpt_systick					=0;
	Dumber.cpt_inactivity				=0;
	Dumber.acquisition					=FALSE;
	Dumber.busyState 						=FALSE;
	Dumber.stateBattery         =2;
};

/*
 *	Calcule le checksum entre le debut du tableau jusqu'à \r
 */

/**
	* Configure le systick à 100ms (1/10éme d'une seconde)
	*/


void Configure_SysTick(void)
{
	SysTick_Config(SystemCoreClock / 20); //configuration du systick à 50ms
}

/**
	*	@brief Désactive les interuptions et entre dans une boucle while (1) en attendant l'extinction du CPU.
	*	@param Aucun paramétre	 
	*/

void shutDown(void)
{
	__disable_irq();
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	while(1);
}

/**
	* @brief Interuption systick. Gestion des timers interne.	
	*/

void SysTick_Handler(void)
{
	
	Dumber.cpt_systick+=50;
	if((Dumber.WatchDogStartEnable == TRUE) && (Dumber.StateSystem != IDLE))
	{
		Dumber.cpt_watchdog+=50;
	}
	Dumber.cpt_inactivity+=50;
	if(asservissement == 1)
	{
		regulation_vitesseD =1;
		regulation_vitesseG =1;
	}
	if(Dumber.cpt_systick%TIMER_1s==0)
	{	
		Dumber.cpt_systick=0;
	}
	
	if(Dumber.cpt_watchdog % 1000 >=250 || Dumber.cpt_watchdog %1000 <=250)
	{
		watchDogState=TRUE;
	}
	else
	{
		watchDogState=FALSE;
	}
	
	if(Dumber.cpt_systick%500 ==0)
	{
		Dumber.acquisition=VOLTAGE;
		voltagePrepare();
	}
	
	if(Dumber.cpt_systick%100==0)
	{
		etatLED++;
		if(etatLED ==12)
			etatLED = 1;
	}
	
	if(Dumber.cpt_inactivity>=120000)
	{
		shutDown();
	}
	
	/*
	  CLIGNOTEMENT : IDLE / DISABLE : CLIGNOTE : RUN ou LOW non clignotant
	  COULEUR :  % BATTERIE en 3 niveau entre 100 et 50 : VERT / entre 50 et 20 : ORANGE / <20 rouge
	*/
	

	
	if(Dumber.cpt_watchdog>=TIMER_Watchdog)
	{
		Dumber.StateSystem=DISABLE;
		cmdRightMotor(BRAKE,0);
		cmdLeftMotor(BRAKE,0);
	}

}
