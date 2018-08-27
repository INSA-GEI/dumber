#ifndef cmde_usart_H
#define cmde_usart_H

#include "stm32f10x.h"

#define TBuffer   30


/* Prototype des fonctions*/
// Intialise l'uart
void INIT_IT_UsartReceive(void);
void INIT_DMASend(void);
void sendDataUSART(void);
void INIT_USART(void);
void MAP_UsartPin(void);

/*Variable Externes necessaire*/

extern char sendString[TBuffer];
extern char receiptString[TBuffer];
extern uint16_t cpt_Rx;
extern uint16_t virgule;
#endif 

