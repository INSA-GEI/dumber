#include "system_dumby.h"
#include "Battery.h"
#include "motor.h"
#include <stm32f10x.h>




uint16_t PrescalerValue = 0;
uint16_t PWM_BATTERY_ON = 0xC0;
uint16_t PWM_BATTERY_OFF = 0;
TIM_TimeBaseInitTypeDef TIM_BaseTempsTimer;
TIM_OCInitTypeDef TIM_PWMConfigure;


ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_BAT_InitStructure;
__IO uint16_t ADCConvertedValue[16];




 /*
	*	@brief Initialise les PIN Necessaire à la mesure de la batterie et à la detection d'une charge
	* EXTI-11 PB11 pour la detection de charge.
	* Pin : A0, A4, A3
	*/

void MAP_batteryPin(void)
{
		GPIO_InitTypeDef Init_Structure;
		NVIC_InitTypeDef NVIC_InitStructure;
		EXTI_InitTypeDef   EXTI_InitStructure;
		/// Variable local necessaire à l'initialisation des structures
	
	
	  /// Configure A3 en output / alternate fonction
		Init_Structure.GPIO_Pin = GPIO_Pin_3;
		Init_Structure.GPIO_Speed = GPIO_Speed_10MHz;
		Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &Init_Structure);
		
		/// Configure les PIN A0,A4 en input floating.
		Init_Structure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_4;
		Init_Structure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOA, &Init_Structure);
	
	
		// Configure PB11 en input floating (à configurer en exti)
		Init_Structure.GPIO_Pin = GPIO_Pin_11;
		Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &Init_Structure);
	
	  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);
	
		EXTI_InitStructure.EXTI_Line = EXTI_Line11;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}




 /*
	* Fonction necessaire au bon fonctionneemnt de la mesure de tension.
  * DMA_BAT : Initialise la dma pour stocké les valeurs dans ADCConvertedValue[]
	* On stockera 16 valeurs de façon à faire un moyennage.
	*/


void DMA_BAT(void)
{
	 /* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_BAT_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR); //   ADC1_DR_Address;
  DMA_BAT_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;
  DMA_BAT_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_BAT_InitStructure.DMA_BufferSize = 16; // voir shcémas ci dessus
  DMA_BAT_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_BAT_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_BAT_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_BAT_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_BAT_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_BAT_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_BAT_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_BAT_InitStructure);
	
	DMA_Cmd(DMA1_Channel1, ENABLE);
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
}


 /*
	* Demmarer acquisition
	*/
void startACQDMA(void)
{
		ADC_DMACmd(ADC1, ENABLE);
		DMA_DeInit(DMA1_Channel1);
		DMA_Init(DMA1_Channel1, &DMA_BAT_InitStructure);
		DMA_Cmd(DMA1_Channel1, ENABLE);
		DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
}
 /*
	* Configuration de l'ADC
	*/
void ADC1_CONFIG(void)
{
 
	/* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel1 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
 // ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2, ADC_SampleTime_55Cycles5);
 
  /* Start ADC1 Software Conversion */ 
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));

  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

	/* voltagePrepare:
	* Demarer acquisition de 16 valeurs de tension
  */
void voltagePrepare(void)
{
	DMA_BAT_InitStructure.DMA_BufferSize = 16;
	//DMA_Init(DMA1_Channel1, &DMA_BAT_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_55Cycles5);
	ADC_Cmd(ADC1, ENABLE);
	startACQDMA();
}

 /* INIT_IT_DMA:
	* Initialise l'IT de fin d'acquisition
	*/
void INIT_IT_DMA(void)
{
	   NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}




 /*DMA1_Channel1_IRQHandler:
	* Interuption DMA pour mettre à jour le flag de calcul de la moyenne des tensions récupérer.
	*/
void DMA1_Channel1_IRQHandler(void)
{
  //Test on DMA1 Channel1 Transfer Complete interrupt
  if(DMA_GetITStatus(DMA1_IT_TC1))
  {
		Dumber.BatterieChecking=TRUE;
   //Clear DMA1 Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits
    DMA_ClearITPendingBit(DMA1_IT_GL1);
		
  }
}

 /*
	* Interuption de detection de chargeur. shutDown le robot;
	*/
void EXTI15_10_IRQHandler(void)
{
	shutDown();
	while(1);
}
