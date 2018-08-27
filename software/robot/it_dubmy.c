#include "system_dumby.h"
#include <stm32f10x.h>

/*
***************************************************************************************************************************************************************
***************************************************			GESTION DU BOUTON OFF			*******************************************************************
***************************************************************************************************************************************************************
*/
void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
		if( Dumber.BatteryOnPlug==FALSE)
     Dumber.BatteryOnPlug=TRUE;
		else
			Dumber.BatteryOnPlug=FALSE;
		
		 EXTI_ClearITPendingBit(EXTI_Line0);
  }
}

/*
***************************************************************************************************************************************************************
**********************************************************GESTIONS DES ENCODEURS PAR INTEURPTIONS**************************************************************
***************************************************************************************************************************************************************
*/

// ENCODEUR RETOUR 
void EXTI15_10_IRQHandler(void)
{
	 if(EXTI_GetITStatus(EXTI_Line11) != RESET)
	 {	 
		  encodeurD1 = TRUE;
		  if(encodeurD2 == TRUE )
		  {
		 		encodeurD1=FALSE;encodeurD2=FALSE;
		 		nbre_tourD--;
		 	}
	 }

	if(EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		 encodeurD2=TRUE;
		if(encodeurD1==TRUE)
		{
			encodeurD1=FALSE;encodeurD2=FALSE;
			nbre_tourD++;
		}
	}	 
		 
		 

	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
		
		 encodeurG1 = TRUE;
		  if(encodeurG2 == 1 )
		  {
		 		encodeurG1=FALSE;encodeurG2=FALSE;
		 		nbre_tourG--;
		 	}
	}	


	if(EXTI_GetITStatus(EXTI_Line14) != RESET)
	{
		encodeurG2 = TRUE;
		if(encodeurG2 == TRUE )
		{
			encodeurG1=FALSE;encodeurG2=FALSE;
			nbre_tourG--;
		}
			
	}
}
