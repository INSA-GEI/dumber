/**
 ******************************************************************************
 * @file    stm32f1xx_it.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    11-February-2014, 2018
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_it.h"
#include "system_dumby.h"

/** @addtogroup IO_Toggle
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                          */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 */
void NMI_Handler(void) {
}

/**
 * @brief  This function handles Hard Fault exception.
 */
void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles Memory Manage exception.
 */
void MemManage_Handler(void) {
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles Bus Fault exception.

 */
void BusFault_Handler(void) {
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 */
void UsageFault_Handler(void) {
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
    }
}

/**
 * @brief  This function handles SVCall exception.
 */
void SVC_Handler(void) {
}

/**
 * @brief  This function handles Debug Monitor exception.
 */
void DebugMon_Handler(void) {
}

/**
 * @brief  This function handles PendSVC exception.
 */
void PendSV_Handler(void) {
}

/**
 * @brief  This function handles SysTick Handler.
 */
/*void SysTick_Handler(void)
{
}*/

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_md.s).                                            */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/

/*
 ***************************************************************************************************************************************************************
 ***************************************************			GESTION DU BOUTON OFF			*******************************************************************
 ***************************************************************************************************************************************************************
 */
/*void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
		if( Dumber.BatteryOnPlug==FALSE)
     Dumber.BatteryOnPlug=TRUE;
		else
			Dumber.BatteryOnPlug=FALSE;

		 EXTI_ClearITPendingBit(EXTI_Line0);
  }
}*/

/*
 ***************************************************************************************************************************************************************
 **********************************************************GESTIONS DES ENCODEURS PAR INTEURPTIONS**************************************************************
 ***************************************************************************************************************************************************************
 */

// ENCODEUR RETOUR
/*void EXTI15_10_IRQHandler(void)
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
}*/

/**
 * @}
 */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
