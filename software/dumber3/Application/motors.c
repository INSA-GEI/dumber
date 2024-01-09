/**
 ******************************************************************************
 * @file motors.c
 * @brief motors driver body
 * @author S. DI MERCURIO (dimercur@insa-toulouse.fr)
 * @date December 2023
 *
 ******************************************************************************
 * @copyright Copyright 2023 INSA-GEI, Toulouse, France. All rights reserved.
 * @copyright This project is released under the Lesser GNU Public License (LGPL-3.0-only).
 *
 * @copyright This file is part of "Dumber" project
 *
 * @copyright This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * @copyright This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * @copyright You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ******************************************************************************
 */

#include "motors.h"
#include "timers.h"

#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_tim.h"

#include <limits.h>

/** @addtogroup Application_Software
  * @{
  */

/** @addtogroup MOTORS
 * Motors driver is in charge of controlling motors and applying a regulation to ensure a linear trajectory
 *
 * Global informations about peripherals
 * - Main clock: 6 Mhz
 * - TIM2 PWM Input (CH1): right encoder PHB : 0 -> 65535
 * - TIM21 PWM Input (CH1): left encoder PHA: 0 -> 65535
 * - TIM3: PWM Output motor (0->200) (~30 Khz)
 * @{
 */

/** @addtogroup MOTORS_Private Private
 * @{
 */

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim21;
extern TIM_HandleTypeDef htim3;

#define MOTORS_MAX_COMMAND	200
#define MOTORS_MAX_ENCODER	USHRT_MAX

/** Structure for storing motore (left or right) regulation state
 * Used during regulation task for controlling motor */
typedef struct {
	int16_t output; 		/**< */
	int16_t set_point;		/**< Xbee RF quality (not used)*/
	uint16_t encoder;		/**< Xbee RF quality (not used)*/
	uint16_t encoderEdge;	/**< Xbee RF quality (not used)*/
	uint8_t slowMotor;		/**< Xbee RF quality (not used)*/
} MOTORS_MotorState;

/** Structure storing counters used for watchdog and system inactivity.
 * Used notably to check if watchdog reset was missed or power down system because of inactivity */
typedef struct {
	uint8_t type;			/**< Xbee RF quality (not used)*/
	int16_t output;		/**< Xbee RF quality (not used)*/
	int16_t set_point;		/**< Xbee RF quality (not used)*/
	int32_t distance;		/**< Xbee RF quality (not used)*/
	int32_t turns;			/**< Xbee RF quality (not used)*/
} MOTORS_DifferentialState;

MOTORS_MotorState MOTORS_LeftMotorState, MOTORS_RightMotorState = { 0 };
MOTORS_DifferentialState MOTORS_DiffState = { 0 };

#define MOTOR_Kp 		300

/***** Tasks part *****/

/* Tache moteurs (gestion des messages) */
StaticTask_t xTaskMotors;
StackType_t xStackMotors[STACK_SIZE];
TaskHandle_t xHandleMotors = NULL;
void MOTORS_HandlerTask(void *params);

/* Tache moteurs périodique (asservissement) */
StaticTask_t xTaskMotorsControl;
StackType_t xStackMotorsControl[STACK_SIZE];
TaskHandle_t xHandleMotorsControl = NULL;
void MOTORS_ControlTask(void *params);

/* Fonctions diverses */
void MOTORS_Set(int16_t leftMotor, int16_t rightMotor);
void MOTORS_PowerOff(void);
void MOTORS_PowerOn(void);
int16_t MOTORS_EncoderCorrection(MOTORS_MotorState state);

#ifdef TESTS
TIM_HandleTypeDef htim7;

volatile uint32_t DEBUG_startTime = 0;
volatile uint32_t DEBUG_endTime = 0;
volatile uint32_t DEBUG_duration = 0;
volatile uint32_t DEBUG_worstCase = 0;

void Init_Systick(void) {
//	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
//
//	__HAL_RCC_TIM7_CLK_ENABLE();
//
//	htim7.Instance = TIM2;
//	htim7.Init.Prescaler = 0;
//	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
//	htim7.Init.Period = 65535;
//	htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//	if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
//		Error_Handler();
//	}
//
//	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
//	if (HAL_TIM_ConfigClockSource(&htim7, &sClockSourceConfig) != HAL_OK) {
//		Error_Handler();
//	}
//
//	LL_TIM_EnableCounter(TIM7);
}

void StartMeasure(void) {
//	DEBUG_startTime = LL_TIM_GetCounter(TIM7);
}

void EndMeasure(void) {
//	DEBUG_endTime = LL_TIM_GetCounter(TIM7);
//
//	if (DEBUG_startTime >= DEBUG_endTime)
//		DEBUG_duration = 65533 - DEBUG_startTime + DEBUG_endTime;
//	else
//		DEBUG_duration = DEBUG_endTime - DEBUG_startTime;
//
//	if (DEBUG_duration > DEBUG_worstCase)
//		DEBUG_worstCase = DEBUG_duration;
}
#endif /* TESTS */

/**
 * @brief  Function for initializing motors driver
 *
 * @param  None
 * @retval None
 */
void MOTORS_Init(void) {
	/* Désactive les alimentations des moteurs */
	MOTORS_PowerOff();

	/* Create the task without using any dynamic memory allocation. */
	xHandleMotors = xTaskCreateStatic(MOTORS_HandlerTask, /* Function that implements the task. */
			"MOTORS Handler", /* Text name for the task. */
			STACK_SIZE, /* Number of indexes in the xStack array. */
			NULL, /* Parameter passed into the task. */
			PriorityMotorsHandler,/* Priority at which the task is created. */
			xStackMotors, /* Array to use as the task's stack. */
			&xTaskMotors); /* Variable to hold the task's data structure. */
	vTaskResume(xHandleMotors);

	/* Create the task without using any dynamic memory allocation. */
	xHandleMotorsControl = xTaskCreateStatic(
			MOTORS_ControlTask, /* Function that implements the task. */
			"MOTORS Control", /* Text name for the task. */
			STACK_SIZE, /* Number of indexes in the xStack array. */
			NULL, /* Parameter passed into the task. */
			PriorityMotorsAsservissement,/* Priority at which the task is created. */
			xStackMotorsControl, /* Array to use as the task's stack. */
			&xTaskMotorsControl); /* Variable to hold the task's data structure. */
	vTaskSuspend(xHandleMotorsControl); // On ne lance la tache d'asservissement que lorsque'une commande moteur arrive

	MOTORS_PowerOff();

#ifdef TESTS
	Init_Systick();
#endif /* TESTS */
}

/**
 * @brief  Function for initializing motors driver
 *
 * @param  None
 * @retval None
 */
void MOTORS_Move(int32_t distance) {
	static int32_t dist;

	dist = distance*15;

	if (dist) {
		MOTORS_PowerOn();
		MESSAGE_SendMailbox(MOTORS_Mailbox, MSG_ID_MOTORS_MOVE,
				APPLICATION_Mailbox, (void*) &dist);
	} else
		MOTORS_Stop();
}

/**
 * @brief  Function for initializing motors driver
 *
 * @param  None
 * @retval None
 */
void MOTORS_Turn(int32_t tours) {
	static int32_t turns;

	turns = tours;

	if (turns) {
		MOTORS_PowerOn();
		MESSAGE_SendMailbox(MOTORS_Mailbox, MSG_ID_MOTORS_TURN,
				APPLICATION_Mailbox, (void*) &turns);
	} else
		MOTORS_Stop();
}

/**
 * @brief  Function for initializing motors driver
 *
 * @param  None
 * @retval None
 */
void MOTORS_Stop(void) {
	MOTORS_PowerOff();
	MESSAGE_SendMailbox(MOTORS_Mailbox, MSG_ID_MOTORS_STOP,
			APPLICATION_Mailbox, (void*) NULL);
}

/*
 * @brief Tache de supervision des moteurs
 * 		  Gestion de la boite aux lettres moteurs, et supervision generale
 * @param params non utilisé
 */
void MOTORS_HandlerTask(void *params) {
	MESSAGE_Typedef msg;
	int32_t distance, tours;

	while (1) {
		msg = MESSAGE_ReadMailbox(MOTORS_Mailbox);

		switch (msg.id) {
		case MSG_ID_MOTORS_MOVE:
			distance = *((int32_t*) msg.data);
			MOTORS_DiffState.distance = distance;
			MOTORS_DiffState.turns = 0;

			if (distance > 0) {
				MOTORS_LeftMotorState.set_point = 50;
				MOTORS_RightMotorState.set_point = 50;
			} else {
				MOTORS_LeftMotorState.set_point = -50;
				MOTORS_RightMotorState.set_point = -50;
			}

			vTaskResume(xHandleMotorsControl);
			break;

		case MSG_ID_MOTORS_TURN:
			tours = *((int32_t*) msg.data);
			MOTORS_DiffState.distance = 0;
			MOTORS_DiffState.turns = tours;

			if (tours > 0) {
				MOTORS_LeftMotorState.set_point = -50;
				MOTORS_RightMotorState.set_point = 50;
			} else {
				MOTORS_LeftMotorState.set_point = 50;
				MOTORS_RightMotorState.set_point = -50;
			}

			vTaskResume(xHandleMotorsControl);
			break;

		case MSG_ID_MOTORS_STOP:
			MOTORS_DiffState.distance = 0;
			MOTORS_DiffState.turns = 0;

			MOTORS_LeftMotorState.set_point = 0;
			MOTORS_RightMotorState.set_point = 0;
			if ((MOTORS_EncoderCorrection(MOTORS_LeftMotorState) == 0)
					&& (MOTORS_EncoderCorrection(MOTORS_RightMotorState) == 0)) {
				// Les moteurs sont déjà arrêtés
				vTaskSuspend(xHandleMotorsControl);

				MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_MOTORS_END_OF_MOUVMENT,
						MOTORS_Mailbox, (void*) NULL);
			} else
				// Les moteurs tournent encore
				vTaskResume(xHandleMotorsControl);
			break;
		default:
			break;
		}
	}
}

/*
 * @brief Tache d'asservissement, périodique (10ms)
 *
 * @param params non utilisé
 */
void MOTORS_ControlTask(void *params) {
	TickType_t xLastWakeTime;
	int16_t leftError, rightError = 0;
	int16_t leftEncoder, rightEncoder;
	int32_t locCmdG, locCmdD;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		// Wait for the next cycle.
		vTaskDelayUntil(&xLastWakeTime,
				pdMS_TO_TICKS(MOTORS_REGULATION_DELAY));

#ifdef TESTS
		//StartMeasure();
#endif /* TESTS */

		leftEncoder = MOTORS_EncoderCorrection(MOTORS_LeftMotorState);
		rightEncoder = MOTORS_EncoderCorrection(MOTORS_RightMotorState);

		/*
		 * encodeur est entre -32768 et +32767, selon le sens de rotation du moteur
		 * consigne est entre -32768 et + 32767 selon le sens de rotation du moteur
		 * erreur est entre -32768 et 32767 selon la difference à apporter à la commande
		 */

		leftError = MOTORS_LeftMotorState.set_point - leftEncoder;
		rightError = MOTORS_RightMotorState.set_point - rightEncoder;

		if (((MOTORS_RightMotorState.set_point == 0)
				&& (MOTORS_LeftMotorState.set_point == 0))
				&& ((rightError == 0) && (leftError == 0))) {

			MOTORS_PowerOff();
			MESSAGE_SendMailbox(APPLICATION_Mailbox, MSG_ID_MOTORS_END_OF_MOUVMENT,
									MOTORS_Mailbox, (void*) NULL);
			vTaskSuspend(xHandleMotorsControl);
		}

		if (MOTORS_LeftMotorState.set_point == 0)
			MOTORS_LeftMotorState.output = 0;
		else {
			if (leftError != 0) {
				//locCmdG = (int32_t)MOTEURS_EtatMoteurGauche.commande + ((int32_t)MOTEUR_Kp*(int32_t)erreurG)/100;
				locCmdG = ((int32_t) MOTOR_Kp * (int32_t) leftError) / 100;

				if (MOTORS_LeftMotorState.set_point >= 0) {
					if (locCmdG < 0)
						MOTORS_LeftMotorState.output = 0;
					else if (locCmdG > SHRT_MAX)
						MOTORS_LeftMotorState.output = SHRT_MAX;
					else
						MOTORS_LeftMotorState.output = (int16_t) locCmdG;
				} else {
					if (locCmdG > 0)
						MOTORS_LeftMotorState.output = 0;
					else if (locCmdG < SHRT_MIN)
						MOTORS_LeftMotorState.output = SHRT_MIN;
					else
						MOTORS_LeftMotorState.output = (int16_t) locCmdG;
				}
			}
		}

		if (MOTORS_RightMotorState.set_point == 0)
			MOTORS_RightMotorState.output = 0;
		else {
			if (rightError != 0) {
				//locCmdD = (int32_t)MOTEURS_EtatMoteurDroit.commande + ((int32_t)MOTEUR_Kp*(int32_t)erreurD)/100;
				locCmdD = ((int32_t) MOTOR_Kp * (int32_t) rightError) / 100;

				if (MOTORS_RightMotorState.set_point >= 0) {
					if (locCmdD < 0)
						MOTORS_RightMotorState.output = 0;
					else if (locCmdD > SHRT_MAX)
						MOTORS_RightMotorState.output = SHRT_MAX;
					else
						MOTORS_RightMotorState.output = (int16_t) locCmdD;
				} else {
					if (locCmdD > 0)
						MOTORS_RightMotorState.output = 0;
					else if (locCmdD < SHRT_MIN)
						MOTORS_RightMotorState.output = SHRT_MIN;
					else
						MOTORS_RightMotorState.output = (int16_t) locCmdD;
				}
			}
		}

		/* Finalement, on applique les commandes aux moteurs */
		MOTORS_Set(MOTORS_LeftMotorState.output,
				MOTORS_RightMotorState.output);

#ifdef TESTS
		//EndMeasure();
#endif /* TESTS */
	}
}

typedef struct {
	uint16_t encoder;
	uint16_t correction;
} MOTORS_CorrectionPoint;

#define MOTORS_MAX_CORRECTION_POINTS 16

const MOTORS_CorrectionPoint MOTORS_CorrectionPoints[MOTORS_MAX_CORRECTION_POINTS] =
{ { MOTORS_MAX_ENCODER - 1, 1 }, { 42000, 100 }, { 22000, 2500 }, {
		18000, 5000 }, { 16500, 7500 }, { 15500, 10000 },
		{ 14500, 12500 }, { 13000, 15000 }, { 12500, 17500 }, { 12200,
				20000 }, { 11500, 22500 }, { 11100, 25000 }, { 11000,
						27500 }, { 10900, 29000 }, { 10850, 30500 }, { 10800,
								SHRT_MAX } // 32767
};

/*
 * @brief Fonction de conversion des valeurs brutes de l'encodeur en valeur linearisées
 *
 * @param encodeur valeur brute de l'encodeur
 * @return valeur linéarisée (entre -32768 et 32767)
 */
int16_t MOTORS_EncoderCorrection(MOTORS_MotorState state) {
	int16_t correction = 0;
	uint8_t index = 0;
	uint32_t A, B, C;
	uint16_t encoder = state.encoder;

	if (encoder == MOTORS_MAX_ENCODER)
		correction = 0;
	else { // recherche par dichotomie de l'intervale
		while (index < MOTORS_MAX_CORRECTION_POINTS) {
			if ((MOTORS_CorrectionPoints[index].encoder >= encoder)
					&& (MOTORS_CorrectionPoints[index + 1].encoder < encoder)) {
				// valeur trouvée, on sort
				break;
			} else
				index++;
		}

		if (index >= MOTORS_MAX_CORRECTION_POINTS)
			correction = SHRT_MAX;
		else {
			A = encoder - MOTORS_CorrectionPoints[index + 1].encoder;
			B = MOTORS_CorrectionPoints[index + 1].correction
					- MOTORS_CorrectionPoints[index].correction;
			C = MOTORS_CorrectionPoints[index].encoder
					- MOTORS_CorrectionPoints[index + 1].encoder;

			correction =
					(int16_t) (MOTORS_CorrectionPoints[index + 1].correction
							- (uint16_t) ((A * B) / C));
		}
	}

	/*
	 * Selon le sens de rotation du moteur (commande > 0 ou < 0), on corrige le signe du capteur
	 */
	if (state.set_point < 0)
		correction = -correction;

	return correction;
}

/**
 * @brief  Function for initializing motors driver
 *
 * @param  None
 * @retval None
 */
void MOTORS_PowerOff(void) {
	LL_TIM_DisableCounter(TIM3);
	LL_TIM_DisableCounter(TIM2);
	LL_TIM_DisableCounter(TIM21);

	LL_TIM_CC_DisableChannel(TIM3,
			LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3
			| LL_TIM_CHANNEL_CH4);

	LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_DisableChannel(TIM21, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2);

	LL_TIM_DisableIT_CC1(TIM2);
	LL_TIM_DisableIT_CC1(TIM21);
	LL_TIM_DisableIT_UPDATE(TIM2);
	LL_TIM_DisableIT_UPDATE(TIM21);

	LL_GPIO_SetOutputPin(GPIOB, SHUTDOWN_ENCODERS_Pin);
	LL_GPIO_ResetOutputPin(GPIOB, SHUTDOWN_5V_Pin);
}

/**
 * @brief  Function for initializing motors driver
 *
 * @param  None
 * @retval None
 */
void MOTORS_PowerOn(void) {
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableCounter(TIM2);
	LL_TIM_EnableCounter(TIM21);

	LL_TIM_CC_EnableChannel(TIM3,
			LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3
			| LL_TIM_CHANNEL_CH4);

	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_EnableChannel(TIM21, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2);

	LL_TIM_EnableIT_CC1(TIM2);
	LL_TIM_EnableIT_CC1(TIM21);
	LL_TIM_EnableIT_UPDATE(TIM2);
	LL_TIM_EnableIT_UPDATE(TIM21);

	LL_GPIO_ResetOutputPin(GPIOB, SHUTDOWN_ENCODERS_Pin);
	LL_GPIO_SetOutputPin(GPIOB, SHUTDOWN_5V_Pin);
}

/**
 * @brief Active les encodeurs et le régulateur des moteur si nécessaire et
 *        règle la commande du moteur (entre -MOTEURS_MAX_COMMANDE et +MOTEURS_MAX_COMMANDE)
 *        On applique une "regle de 3"
 *        pour SHRT_MAX -> MOTEURS_MAX_COMMANDE
 *        pour 0 -> 0
 *        pour une commande C dans l'interval [0 .. 32767], la commande est
 *        	commande = (C * MOTEURS_MAX_COMMANDE)/32767
 *
 * @param[in] cmdGauche blablabla
 * @param[in] cmdDroit blablabla
 */
void MOTORS_Set(int16_t leftMotor, int16_t rightMotor) {
	int32_t leftValue, rightValue;

	leftValue = (int32_t) (((int32_t) leftMotor * (int32_t) SHRT_MAX)
			/ ((int32_t) MOTORS_MAX_COMMAND));
	rightValue = (int32_t) (((int32_t) rightMotor * (int32_t) SHRT_MAX)
			/ ((int32_t) MOTORS_MAX_COMMAND));

	if (LL_GPIO_IsOutputPinSet(GPIOB, SHUTDOWN_5V_Pin) == GPIO_PIN_RESET)
		MOTORS_PowerOn();

	// Moteur droit
	if (rightMotor >= 0) {
		LL_TIM_OC_SetCompareCH2(TIM3, (uint32_t) rightValue);
		LL_TIM_OC_SetCompareCH1(TIM3, (uint32_t) 0);
	} else {
		LL_TIM_OC_SetCompareCH2(TIM3, (uint32_t) 0);
		LL_TIM_OC_SetCompareCH1(TIM3, (uint32_t) rightValue);
	}

	// Moteur gauche
	if (leftMotor >= 0) {
		LL_TIM_OC_SetCompareCH4(TIM3, (uint32_t) leftValue);
		LL_TIM_OC_SetCompareCH3(TIM3, (uint32_t) 0);
	} else {
		LL_TIM_OC_SetCompareCH4(TIM3, (uint32_t) 0);
		LL_TIM_OC_SetCompareCH3(TIM3, (uint32_t) leftValue);
	}
}

/*
 * @brief Recupere les mesures brutes des encodeurs et les enregistre dans la structure moteur correspondante
 *
 * @param[in] htim pointeur sur la reference du timer qui generé l'interruption
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM21) { /* moteur gauche */
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			if (MOTORS_LeftMotorState.slowMotor != 0) {
				MOTORS_LeftMotorState.encoder = MOTORS_MAX_ENCODER;
				MOTORS_LeftMotorState.encoderEdge = MOTORS_MAX_ENCODER;
			} else {
				MOTORS_LeftMotorState.encoder =
						(uint16_t) LL_TIM_IC_GetCaptureCH1(TIM21);
				MOTORS_LeftMotorState.encoderEdge =
						(uint16_t) LL_TIM_IC_GetCaptureCH2(TIM21);
			}

			if (LL_TIM_IsActiveFlag_UPDATE(TIM21))
				LL_TIM_ClearFlag_UPDATE(TIM21);

			MOTORS_LeftMotorState.slowMotor = 0;

			if (MOTORS_DiffState.distance) {
				if (MOTORS_DiffState.distance>0) MOTORS_DiffState.distance--;
				else MOTORS_DiffState.distance++;

				if (MOTORS_DiffState.distance==0) {
					MOTORS_LeftMotorState.set_point=0;
					MOTORS_RightMotorState.set_point=0;
				}
			}

			if (MOTORS_DiffState.turns) {
				if (MOTORS_DiffState.turns>0) MOTORS_DiffState.turns--;
				else MOTORS_DiffState.turns++;

				if (MOTORS_DiffState.turns==0) {
					MOTORS_LeftMotorState.set_point=0;
					MOTORS_RightMotorState.set_point=0;
				}
			}

		}
	} else { /* moteur droit */
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			if (MOTORS_RightMotorState.slowMotor != 0) {
				MOTORS_RightMotorState.encoder = MOTORS_MAX_ENCODER;
				MOTORS_RightMotorState.encoderEdge = MOTORS_MAX_ENCODER;
			} else {
				MOTORS_RightMotorState.encoder =
						(uint16_t) LL_TIM_IC_GetCaptureCH1(TIM2);
				MOTORS_RightMotorState.encoderEdge =
						(uint16_t) LL_TIM_IC_GetCaptureCH2(TIM2);
			}

			if (LL_TIM_IsActiveFlag_UPDATE(TIM2))
				LL_TIM_ClearFlag_UPDATE(TIM2);

			MOTORS_RightMotorState.slowMotor = 0;
		}
	}
}

/*
 * @brief Gestionnaire d'interruption "overflow"
 * 		  Lorsque deux interruptions "overflow" sont arrivées sans que l'interruption capture n'arrive,
 * 		  cela signifie que le moteur est à l'arret.
 * 		  On met la valeur de l'encodeur à MOTEURS_MAX_ENCODEUR
 *
 * @param[in] htim pointeur sur la reference du timer qui generé l'interruption
 */
void MOTORS_TimerEncodeurUpdate(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM21) { /* moteur gauche */
		if ((MOTORS_LeftMotorState.slowMotor++) >= 1) {
			MOTORS_LeftMotorState.encoder = MOTORS_MAX_ENCODER;
			MOTORS_LeftMotorState.slowMotor = 1;
		}
	} else { /* moteur droit */
		if ((MOTORS_RightMotorState.slowMotor++) >= 1) {
			MOTORS_RightMotorState.encoder = MOTORS_MAX_ENCODER;
			MOTORS_RightMotorState.slowMotor = 1;
		}
	}
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
