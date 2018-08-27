#include "stm32f10x.h"                  // Device header
#include <stdio.h>

volatile int32_t ITM_RxBuffer;

void SetLedOn(void);
void SetLedOff(void);
void RCC_Configuration(void);
void GetCurrentAndVoltage(uint32_t *current, uint32_t *voltage);
char VerifyVbatFastCharge(uint32_t voltage);

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define STATE_IDLE 					0x0
#define STATE_FASTCHARGE 		0x01
#define STATE_SLOWCHARGE 		0x02
#define STATE_CHARGEFINISH 	0x03
#define STATE_CALIBRATION   0x10
#define STATE_OFF						0xFF

#define MIN_RATIO 					0
#define MAX_RATIO 					400

#define CURRENT_FASTCHARGE 		0x0C0
#define CURRENT_CHARGEFINISH 	0x00C

#define VERSION "01A"

/* Remarque: Pont diviseur sur 1.5 V pour 9V batterie*/

#define VOLTAGE_IDLE 					0x450  //  5V
#define VOLTAGE_FASTCHARGE 		0x960  //  1.8*6 =>10.80
#define VOLTAGE_ABSOLUTEMAX 	0xA10  //  12V

#define TEMPO_CHARGE_MAX				450*60   /* 45 minutes */		
#define TEMPO_CHARGEFINISH_MAX  12     /* 1.2 seconds */

#define SAFETY_MAX 6

struct MESURE
{
	uint16_t Current;
	uint16_t Voltage;
};

__IO struct MESURE ADCConvertedValue[1024];
 
uint32_t state= STATE_IDLE;
__IO uint8_t tick=0;
uint32_t mesure_vbat;
uint32_t mesure_courant;
uint32_t mincurrent;
uint32_t delta_counter=0;
uint32_t ratio_pwm=0;

uint32_t tempo_chargefinish=0;
uint32_t tempo_charge=0;

uint8_t start=0;
uint8_t cause_exit=0;
uint8_t safety_counter=0;

uint32_t initial_vbat;

uint32_t volatile min_vbat, max_vbat;

void delay(uint32_t time)
{
volatile uint32_t counter = time;
	
	while (counter!=0)
	{
		__nop();
		counter--;
	}
}

/**
  * @brief  Main function, that compute charge algorithms.
  * @param  None
  * @retval None
  */
int main (void)
{
GPIO_InitTypeDef GPIO_InitStructure;
ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;

char count=0;
	
	SysTick_Config(7200000);
	RCC_Configuration();
	
	/* PA8 -> Alternate function (TIM1_CH1) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* PA1 -> Analog input (ADC1_CH1) -> V_Batterie
	   PA2 -> Analog input (ADC1_CH2) -> V_Courant
	*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* PB12 -> Output Push Pull (Act Led) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* ADC_IN1 = PA1 = Mesure_VBAT 
	   ADC_IN2 = PA2 = Mesure_Courant 
	*/
/* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 2*1024;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 2;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_55Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);	
	
	/* TIM1_CH1+CH2 = PA8+PA9 = CMD_MOS */
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 512;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* PWM2 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
  TIM_OCInitStructure.TIM_Pulse = ratio_pwm;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
  TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	
  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
  TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
  TIM_ARRPreloadConfig(TIM1, ENABLE);

  /* TIM1 enable counter */
  TIM_Cmd(TIM1, ENABLE);	
	
	printf ("Let's start ...\n\r");
	
	/* Wait x ms, in order for adc and DMA to acquire some data */
	delay(1000000); // ~ 200 ms
	GetCurrentAndVoltage(&mesure_courant, &initial_vbat);
	
	min_vbat = (uint32_t)(initial_vbat*0.95);
	max_vbat = (uint32_t)(initial_vbat*1.05);
	
	while (1)
	{
		if (tick == 1)
		{
			tick=0;
			count++;
			GetCurrentAndVoltage(&mesure_courant, &mesure_vbat);
			
			VerifyVbatFastCharge(mesure_vbat);
			
			switch (state)
			{
				case STATE_IDLE:

				  if (mesure_vbat >= max_vbat) state = STATE_FASTCHARGE;
				  if (mesure_vbat <= min_vbat) state = STATE_FASTCHARGE;
				  if (mesure_courant !=0) state = STATE_FASTCHARGE;
				
				  tempo_chargefinish=0;
					tempo_charge=0;
				  ratio_pwm = MIN_RATIO;
				  safety_counter=0;
				  mincurrent=0xFFFF;
				  delta_counter=0;
					break;
				
				case STATE_FASTCHARGE:
				
					tempo_charge++;
				 
				  if (mesure_vbat >= VOLTAGE_ABSOLUTEMAX) safety_counter++;
					else safety_counter=0;
							
				  if ((safety_counter >= SAFETY_MAX) || (tempo_charge >= TEMPO_CHARGE_MAX))
					{
						ratio_pwm= MIN_RATIO;
						TIM_SetCompare1(TIM1, (uint16_t)ratio_pwm);
						state = STATE_CHARGEFINISH;
					}
					else
					{
						if (VerifyVbatFastCharge(mesure_vbat) !=0)
						{
							state = STATE_SLOWCHARGE;
						}
						else
						{
							if (mesure_courant>= CURRENT_FASTCHARGE)
							{
								if (ratio_pwm > MIN_RATIO) ratio_pwm--;
							}
							else
							{
								if (ratio_pwm< MAX_RATIO) ratio_pwm ++;
							}
						}
					}
					break;
				
				case STATE_SLOWCHARGE:
					tempo_charge++;
				
				  if (mesure_vbat >= VOLTAGE_ABSOLUTEMAX) safety_counter++;
					else safety_counter=0;
				  
				  if (mesure_courant == 0) 
					{
						state = STATE_CHARGEFINISH;
						ratio_pwm= MIN_RATIO;
						TIM_SetCompare1(TIM1, (uint16_t)ratio_pwm);
					}
					
					if ((safety_counter >= SAFETY_MAX) || (tempo_charge >= TEMPO_CHARGE_MAX))
					{
						state = STATE_CHARGEFINISH;
						ratio_pwm= MIN_RATIO;
						TIM_SetCompare1(TIM1, (uint16_t)ratio_pwm);
						
						if (mesure_vbat >= VOLTAGE_ABSOLUTEMAX) cause_exit=1;
						else cause_exit=2;
					}
					else
					{
						if (mesure_courant<mincurrent) mincurrent=mesure_courant;
						
						/* detection du delta */
						if (mesure_courant>((uint32_t)((float)mincurrent*0.15))+mincurrent)
						{
							delta_counter++;
							
							if (delta_counter ==25)
							{
								ratio_pwm= MIN_RATIO;
								TIM_SetCompare1(TIM1, (uint16_t)ratio_pwm);
							
								state = STATE_CHARGEFINISH;						
								cause_exit=3;
							}
						}
						else delta_counter=0;
						
						if (mesure_vbat >= VOLTAGE_FASTCHARGE)
						{
							if (ratio_pwm > MIN_RATIO) ratio_pwm--;
						}
						else
						{
							if (ratio_pwm< MAX_RATIO) ratio_pwm ++;
						}
						
						if (mesure_courant>= CURRENT_FASTCHARGE)
						{
							if (ratio_pwm > MIN_RATIO) ratio_pwm--;
						}
						
						if ((mesure_courant<= CURRENT_CHARGEFINISH) && (mesure_courant>=2))
						{
							ratio_pwm=MIN_RATIO;
							
							TIM_SetCompare1(TIM1, (uint16_t)ratio_pwm);
							state=STATE_CHARGEFINISH;
							cause_exit=4;
						}
					}
					break;
				
				case STATE_CALIBRATION:
					if (mesure_vbat >= VOLTAGE_FASTCHARGE)
						{
							if (ratio_pwm > MIN_RATIO) ratio_pwm--;
						}
						else
						{
							if (ratio_pwm< MAX_RATIO) ratio_pwm ++;
						}
					break;
				
				case STATE_CHARGEFINISH:
				default:
									
					if (mesure_vbat<=VOLTAGE_IDLE) 
					{
						tempo_chargefinish++;
						
						if (tempo_chargefinish >= TEMPO_CHARGEFINISH_MAX)	state=STATE_IDLE;
					}
					else tempo_chargefinish =0;
					
				  ratio_pwm = MIN_RATIO;
				  
					break;
			}
			
			TIM_SetCompare1(TIM1, (uint16_t)ratio_pwm);

			if (count==10)
			{
				count=0;
				printf("Vbat=0x%X, Icc=0x%X, pwm=%i, state=%i\n\r",mesure_vbat, mesure_courant, ratio_pwm, state);
			}
		}
	}
	
#pragma diag_suppress 111	
	return 0;
}

/**
  * @brief  Compute mean values for current and voltage.
  * @param  current and voltage
  * @retval None
  */
char VerifyVbatFastCharge(uint32_t voltage)
{
	static uint32_t vbatarray[8];
	int i;
	uint32_t accumulator=0;
	
	for (i=1; i<8; i++)
  {
		accumulator = accumulator + vbatarray[i];
		vbatarray[i-1]=vbatarray[i];
	}
	
	vbatarray[7]=voltage;
	accumulator = accumulator+voltage;
	
	accumulator = accumulator>>3;
	
	if(accumulator>=VOLTAGE_FASTCHARGE) return 1;
	else return 0;
	
}

/**
  * @brief  Compute mean values for current and voltage.
  * @param  current and voltage
  * @retval None
  */
void GetCurrentAndVoltage(uint32_t *current, uint32_t *voltage)
{
int i;
uint32_t current_loc=0;
uint32_t voltage_loc=0;
static char firsttime=0;
static uint32_t last_current=0;
static uint32_t last_voltage=0;	

#define MINCURRENT	0x10
#define MAXCURRENT  0x2A
	
#define MINVOLTAGE	0xD0
#define MAXVOLTAGE  0x1D6

uint32_t currentmaxvar, voltagemaxvar;
	
	for (i=0; i<1024; i++)
	{
		current_loc= ADCConvertedValue[i].Current;
		voltage_loc= ADCConvertedValue[i].Voltage;
	}
	
	if (firsttime==0)
	{
		firsttime=1;
		last_current = current_loc;
		last_voltage = voltage_loc;
	}
	else
	{
		currentmaxvar=(uint32_t)((float)last_current*0.1);
		voltagemaxvar=(uint32_t)((float)last_voltage*0.05);
		
		if(currentmaxvar<MINCURRENT) currentmaxvar=MINCURRENT;
		if(currentmaxvar>MAXCURRENT) currentmaxvar=MAXCURRENT;
		
		if(voltagemaxvar<MINVOLTAGE) voltagemaxvar=MINVOLTAGE;
		if(voltagemaxvar>MAXVOLTAGE) voltagemaxvar=MAXVOLTAGE;
		
		if (current_loc>last_current)
		{
			if((current_loc-last_current)>currentmaxvar) current_loc = last_current+currentmaxvar;
		}
		else
		{
			if((last_current-current_loc)>currentmaxvar) current_loc = last_current-currentmaxvar;
		}
		
		if (voltage_loc>last_voltage)
		{
			if((voltage_loc-last_voltage)>voltagemaxvar) voltage_loc = last_voltage+voltagemaxvar;
		}
		else
		{
			if((last_voltage-voltage_loc)>voltagemaxvar) voltage_loc = last_voltage-voltagemaxvar;
		}
		
		last_current = current_loc;
		last_voltage = voltage_loc;
	}
	
	*current=current_loc;
	*voltage=voltage_loc;
}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Configuration(void)
{
	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
  /* GPIOA and GPIOB clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1 | RCC_APB2Periph_TIM1 |
                         RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
}

void SysTick_Handler(void)
{
	static int i=0;
	static int flipflop=0;
	
	i++;
	tick=1;
	
	if (i==10)
	{
		if (state==STATE_FASTCHARGE || state == STATE_SLOWCHARGE)
		{
			if (flipflop==0) 
			{
				SetLedOn();
				flipflop=1;
			}
			else
			{
				SetLedOff();
				flipflop=0;
			}
		}
		else if (state==STATE_CHARGEFINISH) 
		{
			SetLedOn();
			flipflop=0;
		}
		else
		{
			SetLedOff();
			flipflop=0;
		}
		
		i=0;
	}
}

void SetLedOn(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

void SetLedOff(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}








