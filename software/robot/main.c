#include "Battery.h"
#include "system_dumby.h"
#include <stm32f10x.h>
#include "cmde_usart.h"
#include "motor.h"
#include <string.h>
#include "cmde_spi.h"
#include "MAE.h"
#include "led.h"
#include <stdio.h>

float integration1 = 0;
float integration2 = 0;
const float kp = 6;
const float ki = 0.6;
float motD=0,motG=0;
int erreurD;
int erreurG;
char cptMesureHigh=0;
char cptMesureMedium=0;
char cptMesureLow=0;

uint16_t testPostion=0;

/** 
	* @brief Initialise les fréquences des périphérique et du micro. 
	*/
void Configure_Clock_Periph(void)
{
	///Configuration de la fréqyence d'horloge de l'adc*/ 
	RCC_ADCCLKConfig(RCC_PCLK2_Div2); 
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  ///Activation de l'horloge du GPIO, de A B et C, de ADC1, de AFIO
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM2|RCC_APB2Periph_USART1, ENABLE);
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1| RCC_APB2Periph_TIM1 |RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO|RCC_APB2Periph_USART1|RCC_APB2Periph_SPI1, ENABLE);		
}



int main(void)
{	
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
//INIT_TIM3Led();
	INIT_IT_UsartReceive();

	DMA_BAT();
	ADC1_CONFIG();
	INIT_IT_DMA();
	IC_TIM1_CHANEL3();
	IC_TIM1_CHANEL1();
	IT_TIM1();
	GPIO_ResetBits(GPIOA,GPIO_Pin_12); //enable encodeurs
	while(1)
	{	
		uint16_t k;
		uint16_t mesureVoltage;
		if(Dumber.acquisition==VOLTAGE&&Dumber.BatterieChecking==TRUE)
		{
			mesureVoltage=ADCConvertedValue[0];
			for(k=1;k<16;k++)
			{
				mesureVoltage+=ADCConvertedValue[k];
				mesureVoltage/=2;
			}
			if(mesureVoltage < 1650)
				mesureVoltage = 1650;
				mesureVoltage -= 1650;
				mesureVoltage /= 10;
			
			//if(mesureVoltage < Dumber.BatteryPurcentage)
				Dumber.BatteryPurcentage = mesureVoltage;
				Dumber.acquisition=FALSE;
			
			if(Dumber.BatteryPurcentage > 50)
			{
				cptMesureHigh++;
				if(cptMesureHigh >= 15)
				{
					if(Dumber.StateSystem == LOW)
						Dumber.StateSystem = RUN;	
					Dumber.stateBattery = 2;
					cptMesureHigh=0;
					cptMesureMedium=0;
					cptMesureLow=0;
					
				}
			}
			else if (Dumber.BatteryPurcentage < 50 && Dumber.BatteryPurcentage > 10)
			{
				cptMesureMedium++;
				if(cptMesureMedium >= 15)
				{
					if(Dumber.StateSystem == RUN)
						Dumber.StateSystem=LOW;
					Dumber.stateBattery =1;
					cptMesureHigh=0;
					cptMesureMedium=0;
					cptMesureLow=0;
				}
			}
			else
			{
				cptMesureLow++;
				if(cptMesureLow >=15)
				{
					Dumber.StateSystem = DISABLE;
					cptMesureHigh=0;
					cptMesureMedium=0;
					cptMesureLow=0;
					Dumber.stateBattery= 0;
				}
			}
		}
	
		if(regulation_vitesseD)
		{

			erreurD = (signed int)G_speedRight - (signed int)tourD;
			motD = kp * erreurD +integration1;
			integration1 += ki * erreurD;	
			if (motD>255)
				motD=255;
			if (motD<0)
				motD=0;		
			motD=(uint16_t)motD;	
			majVitesseMotorD(motD);	
			tourD = 0;	
			regulation_vitesseD=0;
			
			if(G_lapsRight-tourPositionD < 0)
			{
				cmdRightMotor(BRAKE,255);
			}
		}
		
	if(regulation_vitesseG)
		{	
			erreurG = (signed int)G_speedLeft - (signed int)tourG;
			motG = kp* erreurG + integration2;

			integration2 += ki * erreurG;

			if(motG>255)
				motG=255;
			if(motG<0)
				motG=0;
			motG=(uint16_t)motG;

			majVitesseMotorG(motG);
			tourG = 0;
			regulation_vitesseG=0;

			if(G_lapsLeft-tourPositionG < 0)
			{
				cmdLeftMotor(BRAKE,255);
			}
		}
				
		if(G_lapsLeft-tourPositionG < 0 && G_lapsRight-tourPositionD < 0 && asservissement ==1)
		{
			cmdLeftMotor(BRAKE,255);
			cmdRightMotor(BRAKE,255);
			asservissement = 0;
			erreurD=0;
			erreurG=0;
			integration1=0;
			integration2=0;
			Dumber.busyState=FALSE;
			Dumber.cpt_inactivity = 0;
		}
		
		if(Dumber.StateSystem == IDLE)
		{
			if(etatLED == 1)
			{
				LEDON = 1;
			}
			else if  (etatLED==2)
				LEDON = 0;
		}
		
		if(Dumber.StateSystem == DISABLE)
		{
			if(etatLED %2 == 0)
				LEDred();
			else
				LEDoff();
		}
			
		if(Dumber.StateSystem == RUN || Dumber.StateSystem == LOW)
			LEDON = 1;
		
		if(LEDON)
		{
			if(Dumber.stateBattery==1 && Dumber.StateSystem!=DISABLE)
				LEDorange();
			if(Dumber.stateBattery==2 && Dumber.StateSystem!=DISABLE)
				LEDgreen();
		}
		else
			LEDoff();
		//__WFI();
	}
	#pragma diag_suppress 111
	return 0;

}
	
