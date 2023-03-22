/*
 * xbee_ll.c
 *
 *  Created on: Apr 6, 2022
 *      Author: dimercur
 */

#include "xbee_ll.h"
#include "stm32l4xx_hal.h"

#include "cmsis_os.h"
#include "config.h"

UART_HandleTypeDef XBEE_LL_Uart;
//LPTIM_HandleTypeDef XBEE_LL_Timer;
char* XBEE_LL_RxBuffer;
static TaskHandle_t xbee_ll_rx_thread_handler;
static TaskHandle_t xbee_ll_tx_thread_handler;

typedef enum {
	XBEE_LL_MODE_TRANSPARENT=0,
	XBEE_LL_MODE_API
} XBEE_LL_ModeType;

XBEE_LL_ModeType XBEE_LL_Mode;
FlagStatus XBEE_LL_RxReady;
FlagStatus XBEE_LL_TxReady;

typedef enum
{
	XBEE_LL_RX_STATE_OK=0,
	XBEE_LL_RX_STATE_WAIT_DATA,
	XBEE_LL_RX_STATE_WAIT_HEADER,
	XBEE_LL_RX_STATE_WAIT_EOF,
	XBEE_LL_RX_STATE_TIMEOUT,
	XBEE_LL_RX_STATE_ERROR,
} XBEE_LL_RxStatus;
XBEE_LL_RxStatus XBEE_LL_RXState;

typedef struct {
	uint8_t startChar;
	uint16_t frameLength;
} API_LENGTH_ST;

volatile uint16_t tmp; // to be used by XBEE_LL_ConfigureUart
void XBEE_LL_StartTimeout(uint32_t timeout);
void XBEE_LL_StopTimeout(void);

int XBEE_LL_ConfigureUart(USART_TypeDef* usart, uint32_t baudrate) {
	__HAL_RCC_USART1_FORCE_RESET();
	__HAL_RCC_USART1_RELEASE_RESET();

	XBEE_LL_Uart.Instance = usart;
	XBEE_LL_Uart.Init.BaudRate = baudrate;
	XBEE_LL_Uart.Init.WordLength = UART_WORDLENGTH_8B;
	XBEE_LL_Uart.Init.StopBits = UART_STOPBITS_1;
	XBEE_LL_Uart.Init.Parity = UART_PARITY_NONE;
	XBEE_LL_Uart.Init.Mode = UART_MODE_TX_RX;
	XBEE_LL_Uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	XBEE_LL_Uart.Init.OverSampling = UART_OVERSAMPLING_16;
	XBEE_LL_Uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	XBEE_LL_Uart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	if (HAL_UART_Init(&XBEE_LL_Uart) != HAL_OK)
	{
		return XBEE_LL_ERROR_USART_CFG;
	}

	/* Debug usart */
	tmp = usart->CR1;
	tmp = usart->CR2;
	tmp = usart->CR3;
	tmp = usart->BRR;
	tmp = usart->ICR;
	tmp = usart->ISR;
	tmp = usart->GTPR;
	tmp = usart->RDR;
	tmp = usart->TDR;

	XBEE_LL_TxReady = SET;
	XBEE_LL_RxReady = SET;

	return XBEE_LL_OK;
}

void XBEE_LL_ConfigureGPIO(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/**USART1 GPIO Configuration
		    PA8     ------> XBEE_RESET
		    PA11     ------> XBEE_SLEEP_RQ
		    PB3  -------> XBEE_SLEEP
	 */
	GPIO_InitStruct.Pin = XBEE_RST_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(XBEE_RST_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = XBEE_SLEEP_RQ_PIN;
	HAL_GPIO_Init(XBEE_SLEEP_RQ_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = XBEE_SLEEP_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(XBEE_SLEEP_PORT, &GPIO_InitStruct);

	/* Set SLEEP_REQ pin to 0 (disable sleep mode)
	 *
	 */
	HAL_GPIO_WritePin(XBEE_SLEEP_RQ_PORT, XBEE_SLEEP_RQ_PIN, GPIO_PIN_RESET);
	//HAL_Delay(300);
	vTaskDelay(msToTicks(300));
	/* Reset XBEE module
	 * Pin to 0 -> wait 100 ms -> pin to 1 -> wait 300 ms
	 */
	HAL_GPIO_WritePin(XBEE_RST_PORT, XBEE_RST_PIN, GPIO_PIN_RESET);
	//HAL_Delay(50);
	vTaskDelay(msToTicks(100));

	HAL_GPIO_WritePin(XBEE_RST_PORT, XBEE_RST_PIN, GPIO_PIN_SET);
	//HAL_Delay(400);
	vTaskDelay(msToTicks(400));
}

//void XBEE_LL_ConfigureTimer(void) {
//	/* Peripheral reset */
//	__HAL_RCC_LPTIM1_FORCE_RESET();
//	__HAL_RCC_LPTIM1_RELEASE_RESET();
//
//	XBEE_LL_Timer.Instance = LPTIM1;
//	XBEE_LL_Timer.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
//	XBEE_LL_Timer.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV128;
//	XBEE_LL_Timer.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
//	XBEE_LL_Timer.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
//	XBEE_LL_Timer.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
//	XBEE_LL_Timer.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
//	XBEE_LL_Timer.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
//	XBEE_LL_Timer.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
//
//	HAL_LPTIM_Init(&XBEE_LL_Timer);
//	__HAL_LPTIM_ENABLE(&XBEE_LL_Timer);
//	__HAL_LPTIM_ENABLE_IT(&XBEE_LL_Timer, LPTIM_IT_CMPM);
//	__HAL_LPTIM_AUTORELOAD_SET(&XBEE_LL_Timer, 0xFFFF); /* full scale for timer */
//	__HAL_LPTIM_DISABLE(&XBEE_LL_Timer);
//}

/**
 * @brief  Start the Counter mode in interrupt mode.
 * @param  hlptim LPTIM handle
 * @param  Period Specifies the Autoreload value.
 *         This parameter must be a value between 0x0000 and 0xFFFF.
 * @retval HAL status
 */
//void XBEE_LL_StartTimeout(uint32_t timeout)
//{
//	uint16_t compare_val;
//	uint16_t period;
//
//	if (timeout>0) {
//		/* Enable the Peripheral */
//		__HAL_LPTIM_ENABLE(&XBEE_LL_Timer);
//
//		/* Clear Compare match flag */
//		__HAL_LPTIM_CLEAR_FLAG(&XBEE_LL_Timer,LPTIM_IT_CMPM);
//		/* Load the period value  + current counter in the compare register */
//		period = (uint16_t)((timeout*(4000000UL/128UL))/1000UL);
//
//		if (XBEE_LL_Timer.Instance->CNT>period)
//			compare_val = (uint16_t)(((uint32_t)XBEE_LL_Timer.Instance->CNT)+period-65536);
//		else
//			compare_val = (uint16_t)(((uint32_t)XBEE_LL_Timer.Instance->CNT)+period);
//		__HAL_LPTIM_COMPARE_SET(&XBEE_LL_Timer, compare_val);
//
//		/* Start timer in continuous mode */
//		__HAL_LPTIM_START_CONTINUOUS(&XBEE_LL_Timer);
//	}
//}

/**
 * @brief  Stop the Counter mode in interrupt mode.
 * @param  hlptim LPTIM handle
 * @retval HAL status
 */
//void XBEE_LL_StopTimeout(void)
//{
//	/* Disable the Peripheral */
//	__HAL_LPTIM_DISABLE(&XBEE_LL_Timer);
//
//	/* Clear Compare match flag */
//	__HAL_LPTIM_CLEAR_FLAG(&XBEE_LL_Timer,LPTIM_IT_CMPM);
//}

int XBEE_LL_SendData(char* data, int length) {
	int data_length;

	xbee_ll_tx_thread_handler = xTaskGetCurrentTaskHandle();

	if (length == -1) {
		/* Envoi d'un trame API, donc, recherche de la longueur dans la trame */
		data_length = ((API_LENGTH_ST*)data)->frameLength + 3 +1; //+3 for header and + 1 for checksum
	}
	else data_length = length;

	//while (XBEE_LL_TxReady != SET); // wait for last transfert to end
	ulTaskNotifyTake( pdTRUE, pdMS_TO_TICKS(100)); // wait max 100 ms

	/* Restore huart->gState to ready */
	//XBEE_LL_Uart.gState = HAL_UART_STATE_READY;
	XBEE_LL_TxReady = RESET;
	if(HAL_UART_Transmit_DMA(&XBEE_LL_Uart, (uint8_t*)data, data_length)!= HAL_OK) {
		XBEE_LL_TxReady = SET;
		return XBEE_LL_ERROR_TX;
	}

	xbee_ll_tx_thread_handler = NULL;
	return XBEE_LL_OK;
}

int XBEE_LL_ReceiveData(char* data, int length, int timeout) {
	uint32_t ulNotificationValue;
	int data_length;

	xbee_ll_rx_thread_handler = xTaskGetCurrentTaskHandle();
	while (XBEE_LL_RxReady != SET); // wait for last RX to end

	if (length == -1) {
		// set API mode
		XBEE_LL_Mode = XBEE_LL_MODE_API;
		data_length = 3; // 3 bytes for api header (start char and length)
		XBEE_LL_RxBuffer = data;
		XBEE_LL_RXState = XBEE_LL_RX_STATE_WAIT_HEADER;
	} else {
		// set TRANSPARENT mode
		XBEE_LL_Mode = XBEE_LL_MODE_TRANSPARENT;
		XBEE_LL_RXState = XBEE_LL_RX_STATE_WAIT_DATA;
		data_length = length;
	}

	//XBEE_LL_StartTimeout(timeout);

	XBEE_LL_RxReady = RESET;

	if(HAL_UART_Receive_DMA(&XBEE_LL_Uart, (uint8_t*)data, data_length)!= HAL_OK) {
		//XBEE_LL_StopTimeout();
		HAL_UART_DMAStop(&XBEE_LL_Uart);
		XBEE_LL_RXState = XBEE_LL_RX_STATE_ERROR;
		XBEE_LL_RxReady = SET;

		return XBEE_LL_ERROR_RX;
	}

	//while (XBEE_LL_RxReady != SET); // wait for RX to end
	if (timeout == 0)
		ulNotificationValue = ulTaskNotifyTake( pdTRUE, pdMS_TO_TICKS(portMAX_DELAY)); // wait max 100 ms
	else
		ulNotificationValue = ulTaskNotifyTake( pdTRUE, pdMS_TO_TICKS(timeout)); // wait max 100 ms

	//XBEE_LL_StopTimeout();

	if (ulNotificationValue != 1) {
		/* The reception timed out. */
		HAL_UART_DMAStop(&XBEE_LL_Uart);
		XBEE_LL_RXState = XBEE_LL_RX_STATE_TIMEOUT;
		XBEE_LL_RxReady = SET;
	}

	xbee_ll_rx_thread_handler = NULL;

	if (XBEE_LL_RXState == XBEE_LL_RX_STATE_ERROR)
		return XBEE_LL_ERROR_RX;
	else if (XBEE_LL_RXState == XBEE_LL_RX_STATE_TIMEOUT) {
		HAL_UART_DMAStop(&XBEE_LL_Uart);
		return XBEE_LL_ERROR_RX_TIMEOUT;
	}
	else
		return XBEE_LL_OK;
}

/**
 * @brief  Rx Transfer completed callback
 * @param  UartHandle: UART handle
 * @note   This example shows a simple way to report end of DMA Rx transfer, and
 *         you can add your own implementation.
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	int frame_length;

	if (XBEE_LL_Mode == XBEE_LL_MODE_TRANSPARENT){
		/* Set reception flag: transfer complete*/
		XBEE_LL_RXState = XBEE_LL_RX_STATE_OK;
		XBEE_LL_RxReady = SET;

		if (xbee_ll_rx_thread_handler != NULL) {
			/* Notify the task that an event has been emitted. */
			vTaskNotifyGiveFromISR(xbee_ll_rx_thread_handler, &xHigherPriorityTaskWoken );

			/* There are no more eventin progress, so no tasks to notify. */
			xbee_ll_rx_thread_handler = NULL;

			/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
					    should be performed to ensure the interrupt returns directly to the highest
					    priority task.  The macro used for this purpose is dependent on the port in
					    use and may be called portEND_SWITCHING_ISR(). */
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}
	} else {
		if (XBEE_LL_RXState == XBEE_LL_RX_STATE_WAIT_EOF) {
			/* Set reception flag: transfer complete*/
			XBEE_LL_RXState = XBEE_LL_RX_STATE_OK;
			XBEE_LL_RxReady = SET;

			if (xbee_ll_rx_thread_handler != NULL) {
				/* Notify the task that an event has been emitted. */
				vTaskNotifyGiveFromISR(xbee_ll_rx_thread_handler, &xHigherPriorityTaskWoken );

				/* There are no more eventin progress, so no tasks to notify. */
				xbee_ll_rx_thread_handler = NULL;

				/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
								    should be performed to ensure the interrupt returns directly to the highest
								    priority task.  The macro used for this purpose is dependent on the port in
								    use and may be called portEND_SWITCHING_ISR(). */
				portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
			}
		} else {
			frame_length = 1+(((int)XBEE_LL_RxBuffer[1]<<8))+(int)XBEE_LL_RxBuffer[2];
			XBEE_LL_RXState = XBEE_LL_RX_STATE_WAIT_EOF;

			if(HAL_UART_Receive_DMA(&XBEE_LL_Uart, (uint8_t*)(XBEE_LL_RxBuffer+3), frame_length)!= HAL_OK) {
				// Something went wrong
				//XBEE_LL_StopTimeout();
				XBEE_LL_RXState = XBEE_LL_RX_STATE_ERROR;
				XBEE_LL_RxReady = SET;
			}
		}
	}
}

/**
 * @brief  Tx Transfer completed callback
 * @param  UartHandle: UART handle
 * @note   This example shows a simple way to report end of DMA Rx transfer, and
 *         you can add your own implementation.
 * @retval None
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/* Set transmission flag: transfer complete*/
	XBEE_LL_TxReady = SET;

	/* Restore huart->gState to ready */
	//UartHandle->gState = HAL_UART_STATE_READY;

	if (xbee_ll_tx_thread_handler != NULL) {
		/* Notify the task that an event has been emitted. */
		vTaskNotifyGiveFromISR(xbee_ll_tx_thread_handler, &xHigherPriorityTaskWoken );

		/* There are no more eventin progress, so no tasks to notify. */
		xbee_ll_tx_thread_handler = NULL;

		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
									    should be performed to ensure the interrupt returns directly to the highest
									    priority task.  The macro used for this purpose is dependent on the port in
									    use and may be called portEND_SWITCHING_ISR(). */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

/**
 * @brief This function handles LPTIM1 global interrupt.
 */
//void LPTIM1_IRQHandler(void)
//{
//	/* Clear Compare match flag */
//	__HAL_LPTIM_CLEAR_FLAG(&XBEE_LL_Timer,LPTIM_IT_CMPM);
//
//	XBEE_LL_StopTimeout();
//
//	/* Set reception flag: Timeout*/
//	XBEE_LL_RXState = XBEE_LL_RX_STATE_TIMEOUT;
//	XBEE_LL_RxReady = SET;
//}

