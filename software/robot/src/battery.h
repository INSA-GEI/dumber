/**
 ******************************************************************************
 * @file    battery.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    19-June-2017
 * @brief   Supervision de la tension batterie et detection de charge.
 *
 *			Calcule le voltage de la batterie à interval régulier.
 *			Converti le voltage batterie en signaux de commande - 2 - 1 - 0.
 *			Configure une interruption externe pour détecter le branchement
 *			du chargeur.
 ******************************************************************************
 ******************************************************************************
 */

#ifndef Battery_H
#define Battery_H

#include "stm32f10x.h"

#define VOLTAGE_BUFFER_SIZE 64
extern __IO uint16_t ADCConvertedValue[VOLTAGE_BUFFER_SIZE];

#define VBAT_SEUIL_LOW			0x878
#define VBAT_SEUIL_DISABLE		0x7CA
#define VBAT_SEUIL_EMERGENCY_HALT	0x6E0

void MAP_batteryPin(void);
void DMA_BAT(void);
void ADC1_CONFIG(void);
void INIT_IT_DMA(void);
void startACQDMA(void);
void voltagePrepare(void);

#endif /* Battery_H */

