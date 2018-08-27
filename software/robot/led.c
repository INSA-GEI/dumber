/* Fichier de Gestion des LED*/

#include "system_dumby.h"
#include <stm32f10x.h>
#include "led.h"



TIM_TimeBaseInitTypeDef TIM_TimeBaseLED;
TIM_OCInitTypeDef TIM_OCConfigure;

/**
	*	@brief Cette fonction initialise les E/S du GPIO pour correspondre aux led.
	* @param Aucun
	*/

void MAP_LEDpin(void)
{
		GPIO_InitTypeDef Init_Structure;
	  /// Configure les PIN B0 et B1 en output / alternate fonction
		Init_Structure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
		Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
		Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &Init_Structure);
}

/*void INIT_TIM3Led(void)
{
	// On souhaite une résolution du PWM de 256 valeurs LED TIM3
	TIM_TimeBaseLED.TIM_Period = 255;
	TIM_TimeBaseLED.TIM_Prescaler = 3*3125;
	TIM_TimeBaseLED.TIM_ClockDivision=0;
	TIM_TimeBaseLED.TIM_CounterMode=TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseLED);
	
	// Configuration du PWM sur le timer 2
	TIM_OCConfigure.TIM_OCMode=TIM_OCMode_PWM2; 
	TIM_OCConfigure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCConfigure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCConfigure.TIM_Pulse = 256; // Constante initialisé à 255, Pour un rapport cyclique nul 
	TIM_OCConfigure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OC4Init(TIM3, &TIM_OCConfigure);
	TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);
	
	TIM_OC3Init(TIM3, &TIM_OCConfigure);
	TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	// Enable Counter
	TIM_Cmd(TIM3, ENABLE);
	
	TIM_CtrlPWMOutputs(TIM3,ENABLE);
}

void setLEDGreen(unsigned char pulse)
{
	uint16_t intermediaire;
	intermediaire=255-pulse;
	TIM_SetCompare4(TIM3, intermediaire);
}

void setLEDRed(unsigned char pulse)
{
	uint16_t intermediaire;
	intermediaire=255-pulse;
	TIM_SetCompare3(TIM3, intermediaire);
}
*/

 /*
	* Gestion des LEDs
	*/

void LEDorange(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
	GPIO_SetBits(GPIOB,GPIO_Pin_0);
}

void LEDred(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	GPIO_SetBits(GPIOB,GPIO_Pin_0);
}

void LEDgreen(void)
{
	
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);
}

void LEDoff(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);
}
