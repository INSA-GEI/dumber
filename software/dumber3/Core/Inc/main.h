/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"

#include "config.h"

#include "messages.h"
#include "leds.h"
#include "moteurs.h"
#include "batterie.h"
#include "sequenceur.h"
#include "xbee.h"
#include "cmd.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BATTERY_SENSE_Pin GPIO_PIN_0
#define BATTERY_SENSE_GPIO_Port GPIOA
#define ENC_PHA_GAUCHE_Pin GPIO_PIN_1
#define ENC_PHA_GAUCHE_GPIO_Port GPIOA
#define ENC_PHA_DROIT_Pin GPIO_PIN_2
#define ENC_PHA_DROIT_GPIO_Port GPIOA
#define ENC_PHB_DROIT_Pin GPIO_PIN_3
#define ENC_PHB_DROIT_GPIO_Port GPIOA
#define ENC_PHB_GAUCHE_Pin GPIO_PIN_5
#define ENC_PHB_GAUCHE_GPIO_Port GPIOA
#define PWM_B_GAUCHE_Pin GPIO_PIN_6
#define PWM_B_GAUCHE_GPIO_Port GPIOA
#define PWM_A_GAUCHE_Pin GPIO_PIN_7
#define PWM_A_GAUCHE_GPIO_Port GPIOA
#define PWM_A_DROIT_Pin GPIO_PIN_0
#define PWM_A_DROIT_GPIO_Port GPIOB
#define PWM_B_DROIT_Pin GPIO_PIN_1
#define PWM_B_DROIT_GPIO_Port GPIOB
#define USART_TX_Pin GPIO_PIN_10
#define USART_TX_GPIO_Port GPIOB
#define USART_RX_Pin GPIO_PIN_11
#define USART_RX_GPIO_Port GPIOB
#define LED_ROUGE_Pin GPIO_PIN_12
#define LED_ROUGE_GPIO_Port GPIOB
#define LED_ORANGE_Pin GPIO_PIN_13
#define LED_ORANGE_GPIO_Port GPIOB
#define LED_VERTE_Pin GPIO_PIN_14
#define LED_VERTE_GPIO_Port GPIOB
#define SHUTDOWN_ENCODERS_Pin GPIO_PIN_15
#define SHUTDOWN_ENCODERS_GPIO_Port GPIOB
#define XBEE_RESET_Pin GPIO_PIN_10
#define XBEE_RESET_GPIO_Port GPIOA
#define BUTTON_SENSE_Pin GPIO_PIN_3
#define BUTTON_SENSE_GPIO_Port GPIOB
#define BUTTON_SENSE_EXTI_IRQn EXTI2_3_IRQn
#define USB_SENSE_Pin GPIO_PIN_4
#define USB_SENSE_GPIO_Port GPIOB
#define USB_SENSE_EXTI_IRQn EXTI4_15_IRQn
#define CHARGER_ST2_Pin GPIO_PIN_6
#define CHARGER_ST2_GPIO_Port GPIOB
#define CHARGER_ST1_Pin GPIO_PIN_7
#define CHARGER_ST1_GPIO_Port GPIOB
#define SHUTDOWN_5V_Pin GPIO_PIN_8
#define SHUTDOWN_5V_GPIO_Port GPIOB
#define SHUTDOWN_Pin GPIO_PIN_9
#define SHUTDOWN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
