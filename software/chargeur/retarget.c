/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/
#include <stdio.h>
#include <rt_misc.h>
#include "stm32f10x.h"                  // Device header

#pragma import(__use_no_semihosting_swi)

extern int  sendchar(int ch);  /* in serial.c */

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;

int fputc(int ch, FILE *f) {
#if DEBUG_ITM
  return (ITM_SendChar(ch));
#else
	return 0;
#endif /* DEBUG_ITM */
}

int fgetc (FILE *f)
{
#if DEBUG_ITM
	while (ITM_CheckChar()==0) 
	{
	}
	
	return ITM_ReceiveChar();
#else
	return 0;
#endif /* DEBUG_ITM */
}

int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}

void _ttywrch(int ch) {
  ITM_SendChar(ch);
}

void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
