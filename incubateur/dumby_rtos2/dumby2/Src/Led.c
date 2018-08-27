#include "Led.h"
#include "tim.h"
#include "common.h"

/*Global Variable*/

/*Local Variable*/

/*Local Prototype*/

void selectColor(uint8_t color);

/*Function definition*/


void LED_INIT(void){
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);
}

void LED_SET(uint8_t color, uint8_t dutyCycle){
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);
	selectColor(color);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3,dutyCycle);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4,dutyCycle);
	
}

void selectColor(uint8_t color){
	if(color == LED_GREEN){
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);
		HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_3);
	}
	else if (color == LED_ORANGE){
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
	}
	else if (color == LED_RED){
		HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_4);
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
	}
	else{
		HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_4);
		HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_3);
	}
}
