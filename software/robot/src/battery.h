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

#ifndef _BATTERY_H_
#define _BATTERY_H_

#include "stm32f10x.h"

#define VOLTAGE_BUFFER_SIZE         64

#define VBAT_SEUIL_LOW			    0x878
#define VBAT_SEUIL_DISABLE		    0x7CA
#define VBAT_SEUIL_EMERGENCY_HALT	0x6E0

#define COMPTEUR_SEUIL_HIGH         8
#define COMPTEUR_SEUIL_LOW          8
#define COMPTEUR_SEUIL_DISABLE      8
#define COMPTEUR_SEUIL_EMERGENCY_HALT   3000

extern char cptMesureHigh;
extern char cptMesureLow;
extern char cptMesureDisable;

extern uint16_t vbatLowerVal;
extern uint16_t vbatHighVal;
extern uint16_t vbatDiff;

extern uint16_t testPostion;
extern uint32_t mesureVoltage;
extern uint32_t meanVoltage;

extern uint32_t cptMesureEmergencyHalt;

void batteryConfigure(void);
void batteryManagement(void);
void batteryStartAcquisition(void);
void batteryRefreshData(void);

#endif /* _BATTERY_H_ */

