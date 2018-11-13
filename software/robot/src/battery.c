/**
 ******************************************************************************
 * @file    battery.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    16-mai-2016
 * @brief   Supervision de la tension batterie et detection de charge.
 *			Calcule le voltage de la batterie à interval régulier.
 *			Converti le voltage batterie en signaux de commande - 2 -1 - 0.
 *			Configure une interruption externe pour détecter le branchement
 *			du chargeur.
 ******************************************************************************
 ******************************************************************************
 */
#include <stm32f10x.h>
#include "battery.h"
#include "system_dumby.h"

DMA_InitTypeDef DMA_BAT_InitStructure;
uint16_t ADCConvertedValue[VOLTAGE_BUFFER_SIZE];

char cptMesureHigh=0;
char cptMesureLow=0;
char cptMesureDisable=0;

uint16_t vbatLowerVal;
uint16_t vbatHighVal;
uint16_t vbatDiff;

uint16_t testPostion=0;
uint32_t mesureVoltage;
uint32_t meanVoltage;

uint32_t cptMesureEmergencyHalt=0;

/** @addtogroup Projects
 * @{
 */

/** @addtogroup battery
 * @{
 */


/** @addtogroup Init_GPIO_DMA_IT_Battery
 * @{
 */

/**
 * @brief 		Défini les GPIO nécessaires pour la batterie.
 *
 * 				La fonction MAP_MotorPin va venir configurer le E/S du GPIO pour correspondre avec
 * 				le schéma electrique en ressource. La fonction initialise aussi l'interruption EXTI
 * 				de la détection du chargeur.
 *
 * @note		A3 en output alternate function.
 *				A0 et A4 en floating input.
 *				PB11 en floating input (EXTI)
 *
 * @param  		None
 * @retval 		None
 *
 */

void batteryConfigure(void)
{
    GPIO_InitTypeDef Init_Structure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    Init_Structure.GPIO_Pin = GPIO_Pin_3;
    Init_Structure.GPIO_Speed = GPIO_Speed_10MHz;
    Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &Init_Structure);

    // Configure les PIN A0,A4 en input floating.
    Init_Structure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_4;
    Init_Structure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &Init_Structure);

    // Configure PB11 en input pullup
    Init_Structure.GPIO_Pin = GPIO_Pin_11;
    Init_Structure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &Init_Structure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);

    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Initialise la dma pour stocker les valeur dans ADCConvertedValue.

    DMA_DeInit(DMA1_Channel1);
    DMA_BAT_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR); //   ADC1_DR_Address;
    DMA_BAT_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;
    DMA_BAT_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_BAT_InitStructure.DMA_BufferSize = VOLTAGE_BUFFER_SIZE; // voir schèmas ci dessus
    DMA_BAT_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_BAT_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_BAT_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_BAT_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_BAT_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_BAT_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_BAT_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_BAT_InitStructure);

    DMA_Cmd(DMA1_Channel1, ENABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

    // Configuration et Calibration de l'ADC1 sur 1 channel.
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* ADC1 regular channel1 configuration */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_71Cycles5);

    /* Start ADC1 Software Conversion */
    ADC_Cmd(ADC1, ENABLE);

    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    /* Enable the ADC1 DMA Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @}
 */

/** @addtogroup Lancer_acquisition
 * @{
 */
/**
 * @brief 		Demarrage des Acquisitions de la DMA.
 *
 * @param  		None
 * @retval 		None
 *
 */

void batteryStartAcquisition(void)
{
    ADC_DMACmd(ADC1, ENABLE);
    DMA_DeInit(DMA1_Channel1);
    DMA_Init(DMA1_Channel1, &DMA_BAT_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
}

/**
 * @brief 		Fonction de plus haut niveau qui initialisera la DMA  et qui lancera une nouvelle acquisition.
 * 				Cette fonction est appelée à intervalle régulier dans le systick. Cette fonction utilise startACQDMA.
 *
 * @param  		None
 * @retval 		None
 *
 */

void batteryRefreshData(void)
{
    DMA_BAT_InitStructure.DMA_BufferSize = VOLTAGE_BUFFER_SIZE;

    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_55Cycles5);
    ADC_Cmd(ADC1, ENABLE);
    batteryStartAcquisition();
}

/**
 * @brief       Appelé de manière régulière pour mettre à jour le niveau batterie
 *
 * @param       None
 * @retval      None
 */
void batteryManagement(void) {
    int k;

    if(Dumber.acquisition==VOLTAGE && Dumber.BatterieChecking==TRUE) {
        vbatLowerVal = 0xFFF;
        vbatHighVal = 0;

        for(k=0; k<VOLTAGE_BUFFER_SIZE; k++)
        {
            meanVoltage+=ADCConvertedValue[k];

            if (vbatLowerVal> ADCConvertedValue[k]) vbatLowerVal = ADCConvertedValue[k];
            if (vbatHighVal< ADCConvertedValue[k]) vbatHighVal = ADCConvertedValue[k];
        }

        vbatDiff = vbatHighVal - vbatLowerVal;

        meanVoltage= meanVoltage/VOLTAGE_BUFFER_SIZE;
        mesureVoltage = meanVoltage;

        Dumber.BatteryPercentage = mesureVoltage;
        Dumber.acquisition=FALSE;

        if(Dumber.BatteryPercentage >= VBAT_SEUIL_LOW)
        {
            cptMesureHigh++;
            if(cptMesureHigh >= COMPTEUR_SEUIL_HIGH)
            {
                if(Dumber.StateSystem == STATE_LOW)
                    systemChangeState(STATE_RUN);

                Dumber.stateBattery = 2;
                cptMesureHigh=0;
                cptMesureLow=0;
                cptMesureDisable=0;
                cptMesureEmergencyHalt=0;
            }
        }
        else if (Dumber.BatteryPercentage < VBAT_SEUIL_LOW && Dumber.BatteryPercentage >= VBAT_SEUIL_DISABLE)
        {
            cptMesureLow++;
            if(cptMesureLow >= COMPTEUR_SEUIL_LOW)
            {
                if(Dumber.StateSystem == STATE_RUN)
                    systemChangeState(STATE_LOW);

                Dumber.stateBattery =1;
                cptMesureHigh=0;
                cptMesureLow=0;
                cptMesureDisable=0;
            }
        }
        else // Dumber.BatteryPercentage < VBAT_SEUIL_DISABLE
        {
            cptMesureDisable++;

            if(cptMesureDisable >= COMPTEUR_SEUIL_DISABLE)
            {
                systemChangeState(STATE_DISABLE);
            }
        }
    }
}

/**
 * @}
 */



/** @addtogroup Handler
 * @{
 */


/**
 * @brief 		Interruption Handler. Qui va faire la moyenne des dernières
 * 				acquisitions lorsque la DMA à rempli son buffer.
 *
 * @param  		None
 * @retval 		None
 *
 */
void DMA1_Channel1_IRQHandler(void)
{
    //Test on DMA1 Channel1 Transfer Complete interrupt
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        Dumber.BatterieChecking=TRUE;
        //Clear DMA1 Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits
        DMA_ClearITPendingBit(DMA1_IT_GL1);
    }
}

/**
 * @brief 		Interruption qui donne l'ordre d'éteindre le robot.
 * 				L'IT se déclenche lorsque le chargeur est branché.
 *
 * @param  		None
 * @retval 		None
 *
 */
void EXTI15_10_IRQHandler(void)
{
    systemShutDown();
    while (1);
}

/**
 * @}
 */

/**
 * @}
 */
