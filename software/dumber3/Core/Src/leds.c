/*
 * leds.h
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "main.h"

#define LEDS_AllumeVerte() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET)
#define LEDS_AllumeJaune() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)
#define LEDS_AllumeRouge() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define LEDS_EteintVerte() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET)
#define LEDS_EteintJaune() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)
#define LEDS_EteintRouge() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

LEDS_State LEDS_Animation;
LEDS_State LEDS_AnimationAncien;

StaticTask_t xTaskLeds;

/* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
StackType_t xStackLeds[ STACK_SIZE ];
TaskHandle_t xHandleLeds = NULL;

void LEDS_AnimationThread(void* params);

void LEDS_Init(void) {
	LEDS_EteintVerte();
	LEDS_EteintJaune();
	LEDS_EteintRouge();

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

void LEDS_AnimationThread(void* params) {
	MESSAGE_Typedef msg;
	uint8_t cnt=0;
	static int oneShot=1;

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(100));

		msg = MESSAGE_ReadMailboxNoDelay(LEDS_Mailbox);
		cnt++;

		if (msg.id == MSG_ID_LED_ETAT) { // Si c'est bien un message de changement d'etat LEDS
			LEDS_Animation = *((LEDS_State*)msg.data);

			if (LEDS_Animation != LEDS_AnimationAncien) { 	// si le nouvel etat est different de l'ancien
				// dans ce cas, on eteint les leds pour repartir sur une base saine
				LEDS_AnimationAncien = LEDS_Animation;

				LEDS_EteintVerte();
				LEDS_EteintJaune();
				LEDS_EteintRouge();
				cnt=0;
			}

			if ((LEDS_Animation == cmd_rcv_ok) || (LEDS_Animation == cmd_rcv_err) || (LEDS_Animation == cmd_rcv_unknown))
			{
				oneShot =1;
				cnt=0;
			}
		}

		switch (LEDS_Animation) {
		case leds_off:
			LEDS_EteintVerte();
			LEDS_EteintJaune();
			LEDS_EteintRouge();
			break;
		case leds_idle:
			if (cnt<5)
				LEDS_AllumeVerte();
			else if (cnt<10)
				LEDS_EteintVerte();
			else
				cnt=0;
			break;
		case leds_connecte:
			LEDS_AllumeVerte();
			break;
		case leds_watchdog_expire:
			if (cnt<5)
				LEDS_AllumeRouge();
			else if (cnt<10)
				LEDS_EteintRouge();
			else
				cnt=0;
			break;
		case leds_niveau_bat_0:
			if (!(cnt%2))
				LEDS_AllumeRouge();
			else
				LEDS_EteintRouge();
			break;
		case leds_niveau_bat_1:
			LEDS_AllumeRouge();
			break;
		case leds_niveau_bat_2:
			LEDS_AllumeRouge();

			if (cnt<3)
				LEDS_AllumeJaune();
			else if (cnt<6)
				LEDS_EteintJaune();
			else
				cnt=0;
			break;
		case leds_niveau_bat_3:
			LEDS_AllumeRouge();
			LEDS_AllumeJaune();
			break;
		case leds_niveau_bat_4:
			LEDS_AllumeRouge();
			LEDS_AllumeJaune();

			if (cnt<3)
				LEDS_AllumeVerte();
			else if (cnt<6)
				LEDS_EteintVerte();
			else
				cnt=0;
			break;
		case leds_niveau_bat_5:
			LEDS_AllumeRouge();
			LEDS_AllumeJaune();
			LEDS_AllumeVerte();
			break;
		case leds_charge_bat_0:
		case leds_charge_bat_1:
			if (cnt<3)
				LEDS_AllumeRouge();
			else if (cnt<6)
				LEDS_AllumeJaune();
			else if (cnt<9)
				LEDS_AllumeVerte();
			else if (cnt<12){
				LEDS_EteintVerte();
				LEDS_EteintJaune();
				LEDS_EteintRouge();
			} else
				cnt=0;
			break;
		case leds_charge_bat_2:
		case leds_charge_bat_3:
			LEDS_AllumeRouge();
			if (cnt<3)
				LEDS_AllumeJaune();
			else if (cnt<6)
				LEDS_AllumeVerte();
			else if (cnt <9){
				LEDS_EteintVerte();
				LEDS_EteintJaune();
			} else
				cnt=0;
			break;
		case leds_charge_bat_4:
		case leds_charge_bat_5:
			LEDS_AllumeRouge();
			LEDS_AllumeJaune();

			if (cnt<3)
				LEDS_AllumeVerte();
			else if (cnt<6)
				LEDS_EteintVerte();
			else
				cnt=0;
			break;
		case leds_erreur:
			if (cnt<5) {
				LEDS_AllumeRouge();
				LEDS_AllumeJaune();
				LEDS_AllumeVerte();
			}
			else if (cnt<10) {
				LEDS_EteintRouge();
				LEDS_EteintJaune();
				LEDS_EteintVerte();
			}
			else
				cnt=0;
			break;
		case cmd_rcv_ok:
			if (oneShot) {
				if (cnt<3)
					LEDS_AllumeVerte();
				else if (cnt<10)
					LEDS_EteintVerte();
				else {
					cnt=0;
					oneShot=0;
				}
			}
			break;
		case cmd_rcv_err:
			if (oneShot) {
				if (cnt<3)
					LEDS_AllumeRouge();
				else if (cnt<10)
					LEDS_EteintRouge();
				else {
					cnt=0;
					oneShot=0;
				}
			}
			break;
		case cmd_rcv_unknown:
			if (oneShot) {
				if (cnt<3)
					LEDS_AllumeJaune();
				else if (cnt<10)
					LEDS_EteintJaune();
				else {
					cnt=0;
					oneShot=0;
				}
			}
			break;
		default:
			break;
		}
	}
}
