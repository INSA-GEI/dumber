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
	leds_bat_critical_low,
	leds_bat_low,
	leds_bat_med,
	leds_bat_high,
	leds_bat_charge_low,
	leds_bat_charge_med,
	leds_bat_charge_high,
	leds_bat_charge_complete,
	leds_watchdog_expired,
	leds_erreur_1,
	leds_erreur_2,
	leds_erreur_3,
	leds_erreur_4,
	leds_erreur_5,
	leds_state_unknown
} LEDS_State;

void LEDS_Init(void);
void LEDS_Set(LEDS_State state);

#endif /* INC_LEDS_H_ */
