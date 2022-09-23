/*
 * moteurs.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "main.h"

//extern LPTIM_HandleTypeDef hlptim1;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim21;

int16_t MOTEUR_CmdMoteurG;
int16_t MOTEUR_CmdMoteurD;
int16_t MOTEUR_ConsigneMoteurG;
int16_t MOTEUR_ConsigneMoteurD;

int16_t MOTEUR_NBImpulsionsG;
int16_t MOTEUR_NBImpulsionsD;

uint16_t MOTEUR_DerniereValEncodeursG;
uint16_t MOTEUR_DerniereValEncodeursD;

#define MOTEUR_GAUCHE	0
#define MOTEUR_DROIT	1

#define MOTEUR_Kp 		15
#define MOTEUR_DELAY	3

/*
 * Global informations
 * Main clock: 6 Mhz
 * Tim2 PWM range: 0 - 255 (freq: 23437,5 Mhz)
 * TIM21: encodeur gauche (0 - 65535)
 * LPTIM: encodeur droit (0 - 65535)
 */

/**
 *
 */
void MOTEURS_DesactiveAlim(void) {
	HAL_GPIO_WritePin(GPIOB, SHUTDOWN_ENCODERS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, SHUTDOWN_5V_Pin, GPIO_PIN_RESET);
}

/**
 *
 */
void MOTEURS_ActiveAlim(void) {
	HAL_GPIO_WritePin(GPIOB, SHUTDOWN_ENCODERS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, SHUTDOWN_5V_Pin, GPIO_PIN_SET);
}

/**
 *
 */
GPIO_PinState MOTEURS_EtatAlim(void) {
	return HAL_GPIO_ReadPin(GPIOB, SHUTDOWN_5V_Pin);
}

int16_t MOTEUR_LireEncodeursGauche(void) {
	uint16_t loc_val= htim21.Instance->CNT;
	uint16_t overflow = htim21.Instance->SR;
	int16_t val_end;

	htim21.Instance->SR = htim21.Instance->SR & ~(TIM_SR_UIF);

	if (overflow & TIM_SR_UIF) {
		val_end = 0xFFFF-loc_val + MOTEUR_DerniereValEncodeursG;
	} else {
		val_end = MOTEUR_DerniereValEncodeursG-loc_val;
	}

	MOTEUR_DerniereValEncodeursG = loc_val;
	return val_end;
}

int16_t MOTEUR_LireEncodeursDroit(void) {
//	uint16_t loc_val= hlptim1.Instance->CNT;
//	uint32_t status = hlptim1.Instance->ISR;
//	int16_t val_end;
//
//	hlptim1.Instance->ICR=0xFF; // refait descendre les flags ISR
//
//	if (status & LPTIM_ISR_ARRM) {
//		val_end = 0xFFFF-loc_val + MOTEUR_DerniereValEncodeursD;
//	} else {
//		val_end = MOTEUR_DerniereValEncodeursD-loc_val;
//	}
//
//	val_end= -val_end;
//
//	MOTEUR_DerniereValEncodeursD = loc_val;
//	return val_end;
	return 0;
}

/**
 *
 */
void MOTEURS_Init(void) {
	/* Desactive les alimentations des moteurs */
	MOTEURS_DesactiveAlim();

//	/* Lance les timers (timers PWM + timers encodeurs) et regle tout à zero*/
//	hlptim1.Instance->CR = LPTIM_CR_ENABLE;
//	hlptim1.Instance->CR =  LPTIM_CR_ENABLE | LPTIM_CR_CNTSTRT;
//	hlptim1.Instance->ARR = 65535;
//	hlptim1.Instance->CFGR = LPTIM_CFGR_ENC;
//
//	hlptim1.Instance->CNT = 0;

	htim21.Instance->ARR = 65535;
	htim21.Instance->CR1 = htim21.Instance->CR1 | TIM_CR1_CEN| TIM_CR1_URS;
	htim21.Instance->CNT = 0;

	htim2.Instance->ARR = 255;
	htim2.Instance->CNT = 0;
	htim2.Instance->CCR1 = 0;
	htim2.Instance->CCR2 = 0;
	htim2.Instance->CCR3 = 0;
	htim2.Instance->CCR4 = 0;
	htim2.Instance->CR1 = htim2.Instance->CR1 | TIM_CR1_CEN;
	htim2.Instance->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

	MOTEUR_CmdMoteurG =0;
	MOTEUR_CmdMoteurD =0;
	MOTEUR_ConsigneMoteurG =0;
	MOTEUR_ConsigneMoteurD =0;
}

/**
 * @brief Active les encodeurs et le regulateur des moteur si necessaire et
 *        regle la commande du moteur (entre -255 et +255)
 */
void MOTEURS_Set(uint8_t mot, int16_t val) {
	uint8_t loc_val;

	if (val>=0) {
		if (val>255) loc_val = 255;
		else  loc_val =(uint8_t)val;
	} else {
		if (val < -255) loc_val = 255;
		else loc_val =(uint8_t)(-val);
	}

	if (MOTEURS_EtatAlim()==GPIO_PIN_RESET)
		MOTEURS_ActiveAlim();

	if (mot == MOTEUR_DROIT) {
		if (val >=0) {
			htim2.Instance->CCR1 = (uint16_t)loc_val;
			htim2.Instance->CCR2 = 0;
		} else {
			htim2.Instance->CCR2 = (uint16_t)loc_val;
			htim2.Instance->CCR1 = 0;
		}
	} else {
		if (val >=0) {
			htim2.Instance->CCR4 = (uint16_t)loc_val;
			htim2.Instance->CCR3 = 0;
		} else {
			htim2.Instance->CCR3 = (uint16_t)loc_val;
			htim2.Instance->CCR4 = 0;
		}
	}
}

void MOTEURS_Test (void) {
	int16_t deltaG, deltaD =0;

	MOTEUR_LireEncodeursGauche();
	MOTEUR_LireEncodeursDroit();

	while (1) {
		HAL_Delay(MOTEUR_DELAY);

		MOTEUR_NBImpulsionsG = MOTEUR_LireEncodeursGauche();
		MOTEUR_NBImpulsionsD = MOTEUR_LireEncodeursDroit();

		deltaG = MOTEUR_ConsigneMoteurG - MOTEUR_NBImpulsionsG;
		deltaD = MOTEUR_ConsigneMoteurD - MOTEUR_NBImpulsionsD;

		if (((MOTEUR_ConsigneMoteurD ==0) && (MOTEUR_ConsigneMoteurG ==0)) &&
				((deltaD==0) && (deltaG==0))) MOTEURS_DesactiveAlim();
		else MOTEURS_ActiveAlim();

		if (deltaG !=0) {
			MOTEUR_CmdMoteurG = MOTEUR_CmdMoteurG + MOTEUR_Kp*deltaG;
			if (MOTEUR_ConsigneMoteurG>=0) {
				if (MOTEUR_CmdMoteurG>255) MOTEUR_CmdMoteurG=255;
				if (MOTEUR_CmdMoteurG<0) MOTEUR_CmdMoteurG=0;
			} else {
				if (MOTEUR_CmdMoteurG>0) MOTEUR_CmdMoteurG=0;
				if (MOTEUR_CmdMoteurG<-255) MOTEUR_CmdMoteurG=-255;
			}

			MOTEURS_Set(MOTEUR_GAUCHE, MOTEUR_CmdMoteurG);
		}

		if (deltaD !=0) {
			MOTEUR_CmdMoteurD = MOTEUR_CmdMoteurD + MOTEUR_Kp*deltaD;
			if (MOTEUR_ConsigneMoteurD>=0) {
				if (MOTEUR_CmdMoteurD>255) MOTEUR_CmdMoteurD=255;
				if (MOTEUR_CmdMoteurD<0) MOTEUR_CmdMoteurD=0;
			} else {
				if (MOTEUR_CmdMoteurD>0) MOTEUR_CmdMoteurD=0;
				if (MOTEUR_CmdMoteurD<-255) MOTEUR_CmdMoteurD=-255;
			}

			MOTEURS_Set(MOTEUR_DROIT, MOTEUR_CmdMoteurD);
		}
	}
}
