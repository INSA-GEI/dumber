/*
 * leds.h
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#ifndef INC_LEDS_H_
#define INC_LEDS_H_

typedef enum {
	leds_off=0,
	leds_idle,
	leds_connecte,
	leds_watchdog_expire,
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
	leds_erreur,
	cmd_rcv_ok,
	cmd_rcv_err,
	cmd_rcv_unknown
} LEDS_State;

void LEDS_Init(void);
#endif /* INC_LEDS_H_ */
