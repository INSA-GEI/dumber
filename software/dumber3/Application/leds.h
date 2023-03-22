/*
 * leds.h
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#ifndef INC_LEDS_H_
#define INC_LEDS_H_

#include "application.h"

typedef enum {
	leds_off=0,
	leds_idle,
	leds_run,
	leds_run_with_watchdog,
	leds_niveau_bat_0,
	leds_niveau_bat_1,
	leds_niveau_bat_2,
	leds_niveau_bat_3,
	leds_niveau_bat_4,
	leds_niveau_bat_5,
	leds_charge_bat_0,
	leds_charge_bat_1,
	leds_charge_bat_2,
	leds_charge_bat_3,
	leds_charge_bat_4,
	leds_charge_bat_5,
	leds_erreur_1,
	leds_erreur_2,
	leds_erreur_3,
	leds_erreur_4,
	leds_erreur_5,
	leds_state_unknown
} LEDS_State;

void LEDS_Init(void);
#endif /* INC_LEDS_H_ */
