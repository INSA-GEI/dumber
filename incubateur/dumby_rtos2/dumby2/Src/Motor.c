#include "Motor.h"
#include "tim.h"
#include "common.h"

/*Set Motors Values and regulate them*/

/*Global variables*/
uint32_t coderValue = 0;


/*Local Prototype function*/
void SET_STATEMOTORR(uint8_t state);
void SET_STATEMOTORL(uint8_t state);
uint16_t tempsRoueD;
uint16_t tempsRoueG;
uint16_t distanceR;
uint16_t distanceL;
float speedR;
float speedL;

/*Local variable*/
uint16_t speedSensorR[2];
uint16_t speedSensorL[2];
/*Function*/

void SpeedSensor_Right_Callback(DMA_HandleTypeDef * _hdma);
void SpeedSensor_Left_Callback(DMA_HandleTypeDef * _hdma);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*Init Function*/
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void ENCODER_INIT(void){
	HAL_TIM_Base_Start_IT(&htim1);
	
	//HAL_DMA_RegisterCallback(&hdma_tim1_ch1,0x00, SpeedSensor_Right_Callback);
	//HAL_DMA_RegisterCallback(&hdma_tim1_ch3,0x00, SpeedSensor_Left_Callback);
	HAL_TIM_IC_Start_DMA(&htim1,TIM_CHANNEL_1,(uint32_t *)speedSensorR,2);
	htim1.State = HAL_TIM_STATE_READY;
	HAL_TIM_IC_Start_DMA(&htim1,TIM_CHANNEL_3,(uint32_t *)speedSensorL,2);
	//HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
	//HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
	HAL_GPIO_WritePin(enableEncoders_GPIO_Port,enableEncoders_Pin, GPIO_PIN_RESET);
}

void MOTOR_INIT(uint8_t motor){
	if(motor == MOTOR_RIGHT)	
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	else if(motor == MOTOR_LEFT)
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
		
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*Motor commande function*/
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void MOTORS_SET(uint8_t state, uint16_t speed){
	MOTOR_RIGHT_SET(state, speed);
	MOTOR_LEFT_SET(state, speed);
}

void MOTOR_LEFT_SET(uint8_t state, uint16_t speed){
		SET_STATEMOTORL(state);
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, speed);
}

void MOTOR_RIGHT_SET(uint8_t state, uint16_t speed){
		SET_STATEMOTORR(state);
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, speed);
		
}




/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*Private Function declaration*/
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void SET_STATEMOTORL(uint8_t state){
	switch (state) {
		case MOTOR_BREAK:
			HAL_GPIO_WritePin(cmdStateMotorR_A_GPIO_Port,cmdStateMotorR_A_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(cmdStateMotorR_B_GPIO_Port,cmdStateMotorR_B_Pin,GPIO_PIN_SET);
			break;

		case MOTOR_FORWARD:
			HAL_GPIO_WritePin(cmdStateMotorR_A_GPIO_Port,cmdStateMotorR_A_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(cmdStateMotorR_B_GPIO_Port,cmdStateMotorR_B_Pin,GPIO_PIN_SET);
			break;

		case MOTOR_BACKWARD:
			HAL_GPIO_WritePin(cmdStateMotorR_A_GPIO_Port,cmdStateMotorR_A_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(cmdStateMotorR_B_GPIO_Port,cmdStateMotorR_B_Pin,GPIO_PIN_RESET);
			break;

		default:
			HAL_GPIO_WritePin(cmdStateMotorR_A_GPIO_Port,cmdStateMotorR_A_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(cmdStateMotorR_B_GPIO_Port,cmdStateMotorR_B_Pin,GPIO_PIN_RESET);
		}
}


void SET_STATEMOTORR(uint8_t state){
	switch (state) {
		case MOTOR_BREAK:
			HAL_GPIO_WritePin(cmdStateMotorL_A_GPIO_Port,cmdStateMotorL_A_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(cmdStateMotorL_B_GPIO_Port,cmdStateMotorL_B_Pin,GPIO_PIN_SET);
			break;

		case MOTOR_FORWARD:
			HAL_GPIO_WritePin(cmdStateMotorL_A_GPIO_Port,cmdStateMotorL_A_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(cmdStateMotorL_B_GPIO_Port,cmdStateMotorL_B_Pin,GPIO_PIN_RESET);
			break;

		case MOTOR_BACKWARD:
			HAL_GPIO_WritePin(cmdStateMotorL_A_GPIO_Port,cmdStateMotorL_A_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(cmdStateMotorL_B_GPIO_Port,cmdStateMotorL_B_Pin,GPIO_PIN_SET);
			break;

		default:
			HAL_GPIO_WritePin(cmdStateMotorL_A_GPIO_Port,cmdStateMotorL_A_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(cmdStateMotorL_B_GPIO_Port,cmdStateMotorL_B_Pin,GPIO_PIN_RESET);
		}
}
unsigned int valabs(int val){
	if (val < 0) return -val;
	else return val;
}


uint8_t borneVitesse(int vitesse){
			if(vitesse <= 0)
				return 0;
			else if(vitesse >=100)
				return 100;
			else
				return (uint8_t)vitesse;
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	/*
	 * htim retourne une valeur de channel de 0x01 ou 0x04 selon que l'information vient de la roue droite ou gauche
	 */
	if(htim1.Instance==TIM1 && htim1.Channel==0x01){  // Roue droite
		distanceR++;
		if(speedSensorR[1] - speedSensorR[0] > 0){
			tempsRoueD = speedSensorR[1] - speedSensorR[0];
			if(tempsRoueD <= SPEED_MAX)
				speedR=100;
			else if(tempsRoueD >=SPEED_MIN)
				speedR=0;
			else{
				speedR = (SPEED_MIN-SPEED_MAX) - tempsRoueD;
				speedR = (speedR/(SPEED_MIN-SPEED_MAX))*100;
			}
		}
	}
	if(htim1.Instance==TIM1 && htim1.Channel==0x04){ // Roue gauche
		distanceL++;
		if(speedSensorR[1] - speedSensorR[0] > 0){
			
			tempsRoueG = speedSensorL[1] - speedSensorL[0];
			// Borne de la valeur;
			if(tempsRoueG <= SPEED_MAX)
				speedL=100;
			else if(tempsRoueG >=SPEED_MIN)
				speedL=0;
			else{
				speedL = (SPEED_MIN-SPEED_MAX) - tempsRoueG;
				speedL = (speedL/(SPEED_MIN-SPEED_MAX))*100;
			}
			
			/*if(tempsRoueD>100)
				tempsRoueD=100;*/
		}
	}
}

