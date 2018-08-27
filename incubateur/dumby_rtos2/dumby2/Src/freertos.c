/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * Copyright (c) 2018 STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "sharedData.h"
#include "common.h"
#include "Battery.h"
#include "serviceCom.h"
#include "Led.h"
#include "Xbee.h"
#include "mRequest.h"
#include "Motor.h"
#include <stdlib.h>

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId reqManagerHandle;
osThreadId batteryFilterHandle;
osThreadId ledManagerHandle;
osThreadId extinctionHandle;
osThreadId controlLawHandle;
osThreadId watchDogCtlHandle;
//osMutexId activityCptHandle;
//osMutexId cptWDHandle;
//osMutexId wdEnableHandle;
//osMutexId batteryLvlHandle;
//osMutexId dumbyStateHandle;
//osMutexId consigneHandle;
//osMutexId busyHandle;
//osSemaphoreId synchroMesHandle;

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartReqManager(void const * argument);
void StartBatteryFilter(void const * argument);
void StartLedManager(void const * argument);
void StartExtinction(void const * argument);
void StartControlLow(void const * argument);
void StartWatchDogCtl(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
	return 0;
}
/* USER CODE END 1 */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Create the mutex(es) */
	/* definition and creation of activityCpt */
	osMutexDef(activityCpt);
	activityCptHandle = osMutexCreate(osMutex(activityCpt));

	/* definition and creation of cptWD */
	osMutexDef(cptWD);
	cptWDHandle = osMutexCreate(osMutex(cptWD));

	/* definition and creation of wdEnable */
	osMutexDef(wdEnable);
	wdEnableHandle = osMutexCreate(osMutex(wdEnable));

	/* definition and creation of batteryLvl */
	osMutexDef(batteryLvl);
	batteryLvlHandle = osMutexCreate(osMutex(batteryLvl));

	/* definition and creation of dumbyState */
	osMutexDef(dumbyState);
	dumbyStateHandle = osMutexCreate(osMutex(dumbyState));

	/* definition and creation of consigne */
	osMutexDef(consigne);
	consigneHandle = osMutexCreate(osMutex(consigne));

	/* definition and creation of busy */
	osMutexDef(busy);
	busyHandle = osMutexCreate(osMutex(busy));

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* Create the semaphores(s) */
	/* definition and creation of synchroMes */
	osSemaphoreDef(synchroMes);
	synchroMesHandle = osSemaphoreCreate(osSemaphore(synchroMes), 1);

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* definition and creation of reqManager */
	osThreadDef(reqManager, StartReqManager, osPriorityIdle, 0, 128);
	reqManagerHandle = osThreadCreate(osThread(reqManager), NULL);

	/* definition and creation of batteryFilter */
	osThreadDef(batteryFilter, StartBatteryFilter, osPriorityIdle, 0, 128);
	batteryFilterHandle = osThreadCreate(osThread(batteryFilter), NULL);

	/* definition and creation of ledManager */
	osThreadDef(ledManager, StartLedManager, osPriorityIdle, 0, 128);
	ledManagerHandle = osThreadCreate(osThread(ledManager), NULL);

	/* definition and creation of extinction */
	osThreadDef(extinction, StartExtinction, osPriorityIdle, 0, 128);
	extinctionHandle = osThreadCreate(osThread(extinction), NULL);

	/* definition and creation of controlLaw */
	osThreadDef(controlLaw, StartControlLow, osPriorityHigh, 0, 128);
	controlLawHandle = osThreadCreate(osThread(controlLaw), NULL);

	/* definition and creation of watchDogCtl */
	osThreadDef(watchDogCtl, StartWatchDogCtl, osPriorityAboveNormal, 0, 128);
	watchDogCtlHandle = osThreadCreate(osThread(watchDogCtl), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

	/* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
	/* USER CODE END StartDefaultTask */
}

/* StartReqManager function */
void StartReqManager(void const * argument)
{
	/* USER CODE BEGIN StartReqManager */
	comInit();
	char message[2];
	/* Infinite loop */
	for(;;)
	{
		osSemaphoreWait(synchroMesHandle,osWaitForever);
		setDumberActivity(BUSY);
		message[0] = RxBuffer[8];
		message[1] = manageMessage(RxBuffer);
		xbeeSend(0x0001,message);
	}
	/* USER CODE END StartReqManager */
}

/* StartBatteryFilter function */
void StartBatteryFilter(void const * argument)
{

	/* USER CODE BEGIN StartBatteryFilter */
	BATTERY_INIT();
	char bat;
	/* Infinite loop */
	for(;;)
	{
		bat = BATTERY_FILTER();
		setDumberBattery(bat);
		osDelay(200);
	}
	/* USER CODE END StartBatteryFilter */
}

/* StartLedManager function */
void StartLedManager(void const * argument)
{
	/* USER CODE BEGIN StartLedManager */
	LED_INIT();
	/* Infinite loop */
	for(;;)
	{
		char state = getDumberState();
		char bat = getDumberBattery();

		if(state == DISABLED)
			LED_SET(LED_RED,75);
		else if(state == IDLE){
			if(bat > BATTERY_MEDIUM)
				LED_SET(LED_GREEN,25);
			else if(bat > BATTERY_LOW)
				LED_SET(LED_ORANGE,25);
		}
		else if(state == RUN){
			if(bat > BATTERY_MEDIUM)
				LED_SET(LED_GREEN,122);
			else if(bat > BATTERY_LOW)
				LED_SET(LED_ORANGE, 122);
		}
		if(bat < BATTERY_LOW){
			setDumberState(DISABLED);
		}
		osDelay(200);
	}
	/* USER CODE END StartLedManager */
}

/* StartExtinction function */
void StartExtinction(void const * argument)
{
	/* USER CODE BEGIN StartExtinction */
	int cptTimeOut = TIME_OUT;
	/* Infinite loop */
	// Savoir Lorsqu'un message arrive
	// Décompter le temps entre chaque message
	// si on a un changement d'état sur busy ou si on reçoit un message valide.
	// on remet le compteur à zero
	// Sinon on incrémente le compteur d'inactivité
	for(;;)
	{
		if(cptTimeOut == 0){
			// Shutdown
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
		}
		// On ne décrémente le compteur seulement si le robot est noté comme NOT_BUSY
		// Si celui-ci est BUSY on remet à jour le compteur à TIME_OUT et on reset le compteur

		if(getDumberActivity() == NOT_BUSY)
			cptTimeOut-=100;
		else{
			setDumberActivity(NOT_BUSY);
			cptTimeOut = TIME_OUT;
		}
		osDelay(100);
	}
	/* USER CODE END StartExtinction */
}

/* StartControlLow function */
void StartControlLow(void const * argument)
{
	/* USER CODE BEGIN StartControlLow */
	MOTOR_INIT(MOTOR_LEFT);

	MOTOR_INIT(MOTOR_RIGHT);
	ENCODER_INIT();
	Consigne consigne;
	/* Infinite loop */
	for(;;)
	{
		consigne = getDumberConsigne();
		// BangBang Vitesse
		if(speedL < consigne.motorLSpeed && distanceL < abs(consigne.motorLDistance *7)){
			if(consigne.motorLDistance < 0){
				MOTOR_LEFT_SET(MOTOR_BACKWARD,100);
			}else{
				MOTOR_LEFT_SET(MOTOR_FORWARD,100);
			}

		}else{
			MOTOR_LEFT_SET(MOTOR_FORWARD,0);
		}
		if(speedR < consigne.motorRSpeed && distanceR < abs(consigne.motorRDistance *7) ){
			if(consigne.motorRDistance < 0){
				MOTOR_RIGHT_SET(MOTOR_BACKWARD,100);
			}else{
				MOTOR_RIGHT_SET(MOTOR_FORWARD,100);
			}
		}else{
			MOTOR_RIGHT_SET(MOTOR_FORWARD,0);;
		}

		osDelay(1);
	}
	/* USER CODE END StartControlLow */
}

/* StartWatchDogCtl function */
void StartWatchDogCtl(void const * argument)
{
	/* USER CODE BEGIN StartWatchDogCtl */
	int cptWd;
	/* Infinite loop */
	for(;;)
	{
		if(getDumberWdActive() == WD_ACTIVE){
			cptWd = getDumberCptWd();
			setDumberCptWd(cptWd+5);
			if(cptWd > (WD_BASE_TIME*WD_TRYING_NUMBER)){
				setDumberState(DISABLED);
			}
		}
	    osDelay(5);
	}
	/* USER CODE END StartWatchDogCtl */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
