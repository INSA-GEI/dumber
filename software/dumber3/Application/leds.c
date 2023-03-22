/*
 * leds.h
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "leds.h"

#define LEDS_Allume_Seg_A() HAL_GPIO_WritePin(LED_SEG_A_GPIO_Port, LED_SEG_A_Pin, GPIO_PIN_SET)
#define LEDS_Allume_Seg_B() HAL_GPIO_WritePin(LED_SEG_B_GPIO_Port, LED_SEG_B_Pin, GPIO_PIN_SET)
#define LEDS_Allume_Seg_C() HAL_GPIO_WritePin(LED_SEG_C_GPIO_Port, LED_SEG_C_Pin, GPIO_PIN_SET)
#define LEDS_Allume_Seg_D() HAL_GPIO_WritePin(LED_SEG_D_GPIO_Port, LED_SEG_D_Pin, GPIO_PIN_SET)
#define LEDS_Allume_Seg_E() HAL_GPIO_WritePin(LED_SEG_E_GPIO_Port, LED_SEG_E_Pin, GPIO_PIN_SET)
#define LEDS_Allume_Seg_F() HAL_GPIO_WritePin(LED_SEG_F_GPIO_Port, LED_SEG_F_Pin, GPIO_PIN_SET)
#define LEDS_Allume_Seg_G() HAL_GPIO_WritePin(LED_SEG_G_GPIO_Port, LED_SEG_G_Pin, GPIO_PIN_SET)
#define LEDS_Allume_Seg_DP() HAL_GPIO_WritePin(LED_SEG_DP_GPIO_Port, LED_SEG_DP_Pin, GPIO_PIN_SET)

#define LEDS_Eteint_Seg_A() HAL_GPIO_WritePin(LED_SEG_A_GPIO_Port, LED_SEG_A_Pin, GPIO_PIN_RESET)
#define LEDS_Eteint_Seg_B() HAL_GPIO_WritePin(LED_SEG_B_GPIO_Port, LED_SEG_B_Pin, GPIO_PIN_RESET)
#define LEDS_Eteint_Seg_C() HAL_GPIO_WritePin(LED_SEG_C_GPIO_Port, LED_SEG_C_Pin, GPIO_PIN_RESET)
#define LEDS_Eteint_Seg_D() HAL_GPIO_WritePin(LED_SEG_D_GPIO_Port, LED_SEG_D_Pin, GPIO_PIN_RESET)
#define LEDS_Eteint_Seg_E() HAL_GPIO_WritePin(LED_SEG_E_GPIO_Port, LED_SEG_E_Pin, GPIO_PIN_RESET)
#define LEDS_Eteint_Seg_F() HAL_GPIO_WritePin(LED_SEG_F_GPIO_Port, LED_SEG_F_Pin, GPIO_PIN_RESET)
#define LEDS_Eteint_Seg_G() HAL_GPIO_WritePin(LED_SEG_G_GPIO_Port, LED_SEG_G_Pin, GPIO_PIN_RESET)
#define LEDS_Eteint_Seg_DP() HAL_GPIO_WritePin(LED_SEG_DP_GPIO_Port, LED_SEG_DP_Pin, GPIO_PIN_RESET)

#define LEDS_Eteint_Tout() HAL_GPIO_WritePin(GPIOB, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin, GPIO_PIN_RESET);\
		HAL_GPIO_WritePin(GPIOA, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, GPIO_PIN_RESET)

#define LEDS_Allume_Tout() HAL_GPIO_WritePin(GPIOB, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin, GPIO_PIN_SET);\
		HAL_GPIO_WritePin(GPIOA, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, GPIO_PIN_SET)

#define LEDS_Allume_C() HAL_GPIO_WritePin(GPIOB, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin, GPIO_PIN_SET);\
		HAL_GPIO_WritePin(GPIOA, LED_SEG_D_Pin, GPIO_PIN_SET)

#define LED_PATTERN_ALL_OFF			0
#define LED_PATTERN_BAT_SPRITE_0	1
#define LED_PATTERN_BAT_SPRITE_1	2
#define LED_PATTERN_BAT_SPRITE_2	3
#define LED_PATTERN_BAT_SPRITE_3	4
#define LED_PATTERN_IDLE_0			5
#define LED_PATTERN_IDLE_1			6
#define LED_PATTERN_RUN_0			7
#define LED_PATTERN_RUN_1			8
#define LED_PATTERN_RUN_2			9
#define LED_PATTERN_RUN_3			10
#define LED_PATTERN_RUN_4			11
#define LED_PATTERN_RUN_5			12
#define LED_PATTERN_RUN_WITH_WATCHDOG_0			13
#define LED_PATTERN_RUN_WITH_WATCHDOG_1			14
#define LED_PATTERN_RUN_WITH_WATCHDOG_2			15
#define LED_PATTERN_RUN_WITH_WATCHDOG_3			16
#define LED_PATTERN_RUN_WITH_WATCHDOG_4			17
#define LED_PATTERN_RUN_WITH_WATCHDOG_5			18
#define LED_PATTERN_ERROR			19
#define LED_PATTERN_BATTERY			20
#define LED_PATTERN_DIGIT_0			21
#define LED_PATTERN_DIGIT_1			22
#define LED_PATTERN_DIGIT_2			23
#define LED_PATTERN_DIGIT_3			24
#define LED_PATTERN_DIGIT_4			25
#define LED_PATTERN_DIGIT_5			26
#define LED_PATTERN_DIGIT_6			27
#define LED_PATTERN_DIGIT_7			28
#define LED_PATTERN_DIGIT_8			29
#define LED_PATTERN_DIGIT_9			30
#define LED_PATTERN_DIGIT_UNKNOWN	31

#define LED_MAX_PATTERNS			32

uint16_t LEDS_Patterns [LED_MAX_PATTERNS][4]= {
		// GPIOA ON / GPIOB ON / GPIOA OFF / GPIOB OFF
		{ 0, 0, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin,	LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // All Off
		{ 0, 0, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin,	LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Bat sprite 0
		{ 0, LED_SEG_A_Pin,	LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_B_Pin|LED_SEG_C_Pin}, // Bat sprite 1
		{ LED_SEG_G_Pin, LED_SEG_A_Pin,	LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_DP_Pin, LED_SEG_B_Pin|LED_SEG_C_Pin},   // Bat sprite 2
		{ LED_SEG_D_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin, LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_DP_Pin, LED_SEG_B_Pin|LED_SEG_C_Pin},   // Bat sprite 3
		{ 0, 0, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin,	LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Idle 0
		{ LED_SEG_DP_Pin, 0, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Idle 1
		{ LED_SEG_D_Pin, 0, LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Run 0
		{ LED_SEG_E_Pin, 0, LED_SEG_D_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Run 1
		{ LED_SEG_F_Pin, 0, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Run 2
		{ 0, LED_SEG_A_Pin, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_B_Pin|LED_SEG_C_Pin}, // Run 3
		{ 0, LED_SEG_B_Pin, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_C_Pin}, // Run 4
		{ 0, LED_SEG_C_Pin, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin}, // Run 5
		{ LED_SEG_D_Pin|LED_SEG_DP_Pin, 0, LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Run with watchdog 0
		{ LED_SEG_E_Pin, 0, LED_SEG_D_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Run with watchdog 1
		{ LED_SEG_F_Pin|LED_SEG_DP_Pin, 0, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // Run with watchdog 2
		{ 0, LED_SEG_A_Pin, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_B_Pin|LED_SEG_C_Pin}, // Run with watchdog 3
		{ LED_SEG_DP_Pin, LED_SEG_B_Pin, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_C_Pin},   // Run with watchdog 4
		{ 0, LED_SEG_C_Pin, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin}, // Run with watchdog 5
		{ LED_SEG_D_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin, LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_DP_Pin, 0}, // Error
		{ LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin, LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_DP_Pin, 0}, // Battery
		{ LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin, LED_SEG_G_Pin|LED_SEG_DP_Pin, 0}, // 0
		{ LED_SEG_E_Pin|LED_SEG_F_Pin, 0, LED_SEG_D_Pin|LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // 1
		{ LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin, LED_SEG_F_Pin|LED_SEG_DP_Pin, LED_SEG_C_Pin},   // 2
		{ LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin, LED_SEG_DP_Pin, LED_SEG_B_Pin|LED_SEG_C_Pin},   // 3
		{ LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_C_Pin, LED_SEG_D_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin},   // 4
		{ LED_SEG_D_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_C_Pin, LED_SEG_E_Pin|LED_SEG_DP_Pin, LED_SEG_B_Pin},   // 5
		{ LED_SEG_D_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin, LED_SEG_E_Pin|LED_SEG_DP_Pin},    // 6
		{ LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin, 0, LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin}, // 7
		{ LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_B_Pin|LED_SEG_C_Pin, LED_SEG_DP_Pin, 0}, // 8
		{ LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_F_Pin|LED_SEG_G_Pin, LED_SEG_A_Pin|LED_SEG_C_Pin, LED_SEG_DP_Pin, LED_SEG_B_Pin},   // 9
		{ LED_SEG_D_Pin|LED_SEG_E_Pin|LED_SEG_G_Pin, LED_SEG_B_Pin, LED_SEG_G_Pin|LED_SEG_DP_Pin, LED_SEG_A_Pin|LED_SEG_C_Pin}    // ?
};

LEDS_State LEDS_Animation;
LEDS_State LEDS_AnimationAncien;

StaticTask_t xTaskLeds;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackLeds[ STACK_SIZE ];
TaskHandle_t xHandleLeds = NULL;

void LEDS_AnimationThread(void* params);
void LEDS_ShowPattern(uint8_t pattern);
void LEDS_Tests(void* params);

void LEDS_Init(void) {
	LEDS_Eteint_Tout();

	LEDS_Animation=leds_off;
	LEDS_AnimationAncien =LEDS_Animation;

	/* Mailbox is created in messages.c */

	/* Create the task without using any dynamic memory allocation. */
	xHandleLeds = xTaskCreateStatic(
			LEDS_AnimationThread,       /* Function that implements the task. */
			"LEDS Animation",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityLeds,/* Priority at which the task is created. */
			xStackLeds,          /* Array to use as the task's stack. */
			&xTaskLeds);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleLeds);
}

void LEDS_ShowPattern(uint8_t pattern) {
	if (pattern < LED_MAX_PATTERNS) {
		HAL_GPIO_WritePin(GPIOA, LEDS_Patterns[pattern][2], GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, LEDS_Patterns[pattern][3], GPIO_PIN_RESET);

		if (LEDS_Patterns[pattern][0] + LEDS_Patterns[pattern][1]!=0) {
			HAL_GPIO_WritePin(GPIOA, LEDS_Patterns[pattern][0], GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, LEDS_Patterns[pattern][1], GPIO_PIN_SET);
		}
	}
}

void LEDS_Tests(void* params) {
	LEDS_State ledState = leds_idle;

	LEDS_Eteint_Tout();

	while (1) {
		MESSAGE_SendMailbox(LEDS_Mailbox, MSG_ID_LED_ETAT, LEDS_Mailbox, (void*)&ledState);
		vTaskDelay(pdMS_TO_TICKS(5000)); // wait 10s

		ledState++;
		if (ledState>leds_state_unknown) ledState=leds_idle;
	}
}

void LEDS_AnimationThread(void* params) {
	MESSAGE_Typedef msg;
	uint8_t cnt=0;

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(100));

		msg = MESSAGE_ReadMailboxNoDelay(LEDS_Mailbox);
		cnt++;

		if (msg.id == MSG_ID_LED_ETAT) { // Si c'est bien un message de changement d'etat LEDS
			LEDS_Animation = *((LEDS_State*)msg.data);

			if (LEDS_Animation != LEDS_AnimationAncien) { 	// si le nouvel etat est different de l'ancien
				// dans ce cas, on eteint les leds pour repartir sur une base saine
				LEDS_AnimationAncien = LEDS_Animation;

				LEDS_Eteint_Tout();
				cnt=0;
			}
		}

		switch (LEDS_Animation) {
		case leds_off:
			LEDS_Eteint_Tout();
			break;
		case leds_idle:
			if (cnt<5)
				LEDS_ShowPattern(LED_PATTERN_IDLE_1);
			else if (cnt<10)
				LEDS_ShowPattern(LED_PATTERN_IDLE_0);
			else
				cnt=0;
			break;
		case leds_run:
			if (cnt>5) cnt=0;
			LEDS_ShowPattern(LED_PATTERN_RUN_0+cnt);
			break;
		case leds_run_with_watchdog:
			if (cnt>5) cnt=0;
			LEDS_ShowPattern(LED_PATTERN_RUN_WITH_WATCHDOG_0+cnt);
			break;
		case leds_niveau_bat_0:
			if (!(cnt%2))
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_1);
			else
				LEDS_ShowPattern(LED_PATTERN_ALL_OFF);
			break;
		case leds_niveau_bat_1:
			LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_1);
			break;
		case leds_niveau_bat_2:
			if (cnt<3)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_1);
			else if (cnt<6)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_2);
			else
				cnt=0;
			break;
		case leds_niveau_bat_3:
			if (cnt<3)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_1);
			else if (cnt<6)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_2);
			else
				cnt=7; // on maintient l'affichage de deux barres
			break;
		case leds_niveau_bat_4:
			if (cnt<3)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_1);
			else if (cnt<6)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_2);
			else if (cnt <9)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_3);
			else
				cnt=3;
			break;
		case leds_niveau_bat_5:
			if (cnt<3)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_1);
			else if (cnt<6)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_2);
			else if (cnt <9)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_3);
			else
				cnt=10;
			break;
		case leds_charge_bat_0:
		case leds_charge_bat_1:
			if (cnt<3)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_1);
			else if (cnt<6)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_2);
			else if (cnt<9)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_3);
			else if (cnt<12)
				LEDS_ShowPattern(LED_PATTERN_ALL_OFF);
			else
				cnt=0;
			break;
		case leds_charge_bat_2:
		case leds_charge_bat_3:
			if (cnt<3)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_2);
			else if (cnt<6)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_3);
			else if (cnt <9)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_1);
			else
				cnt=0;
			break;
		case leds_charge_bat_4:
		case leds_charge_bat_5:
			if (cnt<3)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_3);
			else if (cnt<6)
				LEDS_ShowPattern(LED_PATTERN_BAT_SPRITE_2);
			else
				cnt=0;
			break;
		case leds_erreur_1:
			if (cnt<5)
				LEDS_ShowPattern(LED_PATTERN_ERROR);
			else if (cnt<10)
				LEDS_ShowPattern(LED_PATTERN_DIGIT_1);
			else
				cnt=0;
			break;
		case leds_erreur_2:
			if (cnt<5)
				LEDS_ShowPattern(LED_PATTERN_ERROR);
			else if (cnt<10)
				LEDS_ShowPattern(LED_PATTERN_DIGIT_2);
			else
				cnt=0;
			break;
		case leds_erreur_3:
			if (cnt<5)
				LEDS_ShowPattern(LED_PATTERN_ERROR);
			else if (cnt<10)
				LEDS_ShowPattern(LED_PATTERN_DIGIT_3);
			else
				cnt=0;
			break;
		case leds_erreur_4:
			if (cnt<5)
				LEDS_ShowPattern(LED_PATTERN_ERROR);
			else if (cnt<10)
				LEDS_ShowPattern(LED_PATTERN_DIGIT_4);
			else
				cnt=0;
			break;
		case leds_erreur_5:
			if (cnt<5)
				LEDS_ShowPattern(LED_PATTERN_ERROR);
			else if (cnt<10)
				LEDS_ShowPattern(LED_PATTERN_DIGIT_5);
			else
				cnt=0;
			break;
		case leds_state_unknown:
			if (cnt<3)
				LEDS_ShowPattern(LED_PATTERN_DIGIT_UNKNOWN);
			else if (cnt<10)
				LEDS_ShowPattern(LED_PATTERN_ALL_OFF);
			else
				cnt=0;
			break;
		default:
			break;
		}
	}
}
