/*
 * panic.h
 *
 *  Created on: Oct 11, 2023
 *      Author: dimercur
 */

#ifndef PANIC_H_
#define PANIC_H_

typedef enum {
	panic_charger_err=1,  // erreur 1
	panic_adc_err,        // erreur 2
	panic_malloc          // erreur 3
} PANIC_Typedef;

void PANIC_Raise(PANIC_Typedef panicId);

#endif /* PANIC_H_ */
