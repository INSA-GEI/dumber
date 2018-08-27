#include "system_dumby.h"
#include <stm32f10x.h>
#include "motor.h"

/** @Note
	* Ce fichier contient les fonctions lié à la gestions du moteur :
	* - L'initialisation des PIN ultisé par les moteurs et encodeurs
	* - L'initialisation du Timer2 servant de bases de temps aux PWM pour la charge / moteur1 et moteur2
	*	- L'initialisation du Timer3 servant à échantilloner les capteur magnétique de position des 2 moteurs.
	* - La commandes des moteurs dans leurs 3 modes de fonctionnement (AVANT, ARRIERE, FREIN) et de leurs vitesse (valeur de PWM entre 0 et 255)
	*/

/**
	*			TABLE DE VERITEE DU DRIVER MOTEUR
	*
	*	ENABLE  |			INPUTS	|  Moteurs
	*					|							|
	*		0			|				X			|	Roue Libre
	*--------------------------------------
	*					|	CMDA & !CMDB| Avant
	*		1			| !CMDA & CMDB| Arriére
	*					|  CMDA = CMDB| Frein
	*/




__IO uint16_t IC1ReadValue1 = 0, IC1ReadValue2 = 0, IC3ReadValue1 = 0, IC3ReadValue2 = 0;
__IO uint16_t CaptureMotor1Nbr = 0, CaptureMotor2Nbr = 0;
__IO uint32_t motor1Capture, motor2Capture = 0;


	TIM_TimeBaseInitTypeDef TIM2_TempsPWMsettings;
	TIM_OCInitTypeDef TIM2_Configure;

	uint16_t tourD = 0, tourG=0, tourPositionD, tourPositionG;
	// definition des variables global
	uint16_t G_speedRight=20, G_speedLeft=20, G_lapsLeft, G_lapsRight;
/**
	* @brief 			La fonction mapMotorPin va venir configurer le E/S du GPIO pour correspondre avec le schéma electrique en ressource.
	*							2 pwm en alternate fonction : PA1,PA2. 4 entrée timer. PA8,PA9,PA10,PA11. 4 sortie ppull PB12,PB13,PB14,PB15	
	*
	* @param  		Aucun
	* @retval 		Aucun
	*
	*	@Attention 	La definitions de ces E/S ne sera valable que dans la version du prototype n°2 de Dumby. 
	*							Si vous utilisez la versions n°1, appellez mapMotorPinOld().
	*/
	void MAP_MotorPin(void)
	{
		/// Variable local necessaire à l'initialisation des structures
		GPIO_InitTypeDef Init_Structure;
	
	  /// Configure les PIN A1 et A2 en output / alternate fonction
		Init_Structure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
		Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
		//Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
		Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &Init_Structure);
		
		/// Configure les PIN B12,B13,B14, et B15 en output ppull.
		Init_Structure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
		Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
		Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &Init_Structure);
		
		
		/// Configure les PIN A12 en output ppull - enable encodeurs
		Init_Structure.GPIO_Pin = GPIO_Pin_12;
		Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
		Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA, &Init_Structure);
		
		/// Configure les PIN A8,A9,A10, et A11 en input floating.
		Init_Structure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
		Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
		Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &Init_Structure);
	}
	
	
	
	
/**
	* @brief 	La fonction pwmMotorInit() initialise le Timer2 à une fréquence de 25kHz (fc nominal) et initialise 
	*					les pwm des moteurs à un dutycycle à 0% de 255
	*	
	* @param  Aucun
	* @retval Aucun
	*/

void INIT_TIM2(void)
{
	// On souhaite une résolution du PWM de 256 valeurs MOTOR1 TIM2
	TIM2_TempsPWMsettings.TIM_Period = 255;
	TIM2_TempsPWMsettings.TIM_Prescaler = 0;
	TIM2_TempsPWMsettings.TIM_ClockDivision=0;
	TIM2_TempsPWMsettings.TIM_CounterMode=TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM2, &TIM2_TempsPWMsettings);
}
/*
 * Initialisation du chanel
 */
void INIT_OCMotorPwm(void)
{
	// Configuration du PWM sur le timer 2
	TIM2_Configure.TIM_OCMode=TIM_OCMode_PWM2; 
	TIM2_Configure.TIM_OutputState = TIM_OutputState_Enable;
	TIM2_Configure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM2_Configure.TIM_Pulse = 256; // Constante initialisé à 256, Pour un rapport cyclique nul 
	TIM2_Configure.TIM_OCPolarity = TIM_OCPolarity_High;
		
	TIM_OC3Init(TIM2, &TIM2_Configure);
	TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);
	
	TIM_OC2Init(TIM2, &TIM2_Configure);
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	// Enable Counter
	TIM_Cmd(TIM2, ENABLE);
	
	TIM_CtrlPWMOutputs(TIM2,ENABLE);
}


void IC_TIM1_CHANEL1(void)
{
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_ICInit(TIM1, &TIM_ICInitStructure);
	TIM_Cmd(TIM1, ENABLE);
	TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);
}


void IC_TIM1_CHANEL3(void)
{
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_ICInit(TIM1, &TIM_ICInitStructure);
	TIM_Cmd(TIM1, ENABLE);
	TIM_ITConfig(TIM1, TIM_IT_CC3, ENABLE);
}


void IT_TIM1(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the TIM1 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


/**
	* @brief 	La fonction cmdRightMotor() va réglé le moteur droit selon les paramétres d'éntrée.
	*					
	* @param  mode de fonctionnement du moteur, peut être égal aux constantes BRAKE, FORWARD, REVERSE définis dans moteur.h
	* @param 	consigne de vitesse du moteur, définis par un pwm entre 0 et 255.
	*	@retval
	*/					

void cmdRightMotor(char mod, uint16_t pwm)
{
	pwm = 256-pwm;
	switch (mod)
	{
		case BRAKE :	GPIO_SetBits(GPIOB, GPIO_Pin_12);
									GPIO_SetBits(GPIOB, GPIO_Pin_13);
								break;
		case FORWARD :	GPIO_SetBits(GPIOB, GPIO_Pin_12);
										GPIO_ResetBits(GPIOB, GPIO_Pin_13);
								break;
		
		case REVERSE : GPIO_SetBits(GPIOB, GPIO_Pin_13);
									GPIO_ResetBits(GPIOB, GPIO_Pin_12);
								break;
		
		default : 		GPIO_ResetBits(GPIOB, 12);
									GPIO_ResetBits(GPIOB, 13);
	}
		TIM_SetCompare3(TIM2, pwm);
}


/**
	* @brief 	La fonction cmdLefttMotor() va réglé le moteur gauche selon les paramétres d'éntrée.
	*					
	* @param  mode de fonctionnement du moteur, peut être égal aux constantes BRAKE, FORWARD, REVERSE définis dans moteur.h
	* @param 	consigne de vitesse du moteur, définis par un pwm entre 0 et 255.
	*	@retval
	*/	



void cmdLeftMotor(char mod, uint16_t pwm)
{    
	pwm = 256-pwm;
	switch (mod)
	{
		case BRAKE : 	GPIO_SetBits(GPIOB, GPIO_Pin_14);
									GPIO_SetBits(GPIOB, GPIO_Pin_15);
								break;
		case FORWARD :  GPIO_SetBits(GPIOB, GPIO_Pin_15);
										GPIO_ResetBits(GPIOB, GPIO_Pin_14);
								break;
		
		case REVERSE : GPIO_SetBits(GPIOB, GPIO_Pin_14);
									 GPIO_ResetBits(GPIOB, GPIO_Pin_15);
								break;
		
		default :GPIO_ResetBits(GPIOB, GPIO_Pin_14);
						 GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	}
	TIM_SetCompare2(TIM2, pwm);
}


void majVitesseMotorG(uint16_t pwm)
{
		pwm = 256-pwm;
		TIM_SetCompare2(TIM2, pwm);
}

void majVitesseMotorD(uint16_t pwm)
{
		pwm = 256-pwm;
		TIM_SetCompare3(TIM2, pwm);
}



void regulationMoteur(char modRight, char modLeft, uint16_t lapsRight, uint16_t lapsLeft, uint16_t speedRight, uint16_t speedLeft)
{
	/*Moteur Droit*/
	switch (modRight)
	{
		case BRAKE :	GPIO_SetBits(GPIOB, GPIO_Pin_12);
									GPIO_SetBits(GPIOB, GPIO_Pin_13);
								break;
		case FORWARD :	GPIO_SetBits(GPIOB, GPIO_Pin_12);
										GPIO_ResetBits(GPIOB, GPIO_Pin_13);
								break;
		
		case REVERSE : GPIO_SetBits(GPIOB, GPIO_Pin_13);
									GPIO_ResetBits(GPIOB, GPIO_Pin_12);
								break;
		
		default : 		GPIO_ResetBits(GPIOB, 12);
									GPIO_ResetBits(GPIOB, 13);
	}
	
	/* Moteur Gauche */
	switch (modLeft)
	{
		case BRAKE : 	  GPIO_SetBits(GPIOB, GPIO_Pin_14);
									  GPIO_SetBits(GPIOB, GPIO_Pin_15);
										break;
		case FORWARD :  GPIO_SetBits(GPIOB, GPIO_Pin_15);
										GPIO_ResetBits(GPIOB, GPIO_Pin_14);
										break;
		
		case REVERSE :  GPIO_SetBits(GPIOB, GPIO_Pin_14);
									  GPIO_ResetBits(GPIOB, GPIO_Pin_15);
										break;
		
		default :				GPIO_ResetBits(GPIOB, GPIO_Pin_14);
										GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	}
	
	if((speedRight==0 && lapsRight>0) || (speedLeft==0 && lapsLeft>0))
	{
		while(1); // Gestion erreur à coder
	}

	G_lapsLeft = lapsLeft;
	G_speedLeft = speedLeft;
	G_lapsRight = lapsRight;
	G_speedRight= speedRight;
	asservissement = 1;
	tourPositionD=0;
	tourPositionG=0;
}
	

 /*
	* Incremente le nombre de passage devant les capteurs à chaque interuption de ceux ci.
	*/
void TIM1_CC_IRQHandler(void)
{ 
  if(TIM_GetITStatus(TIM1, TIM_IT_CC1) == SET) 
  {
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
		tourD++;
		tourPositionD++;

  }
	
  if(TIM_GetITStatus(TIM1, TIM_IT_CC3) == SET) 
  {
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC3);
		tourG++;
		tourPositionG++;
  }
}
