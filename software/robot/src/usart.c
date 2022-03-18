/**
 ******************************************************************************
 * @file    cmd_uart.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    16-mai-2016
 * @brief   Gestion de l'uart
 * @note  Ce fichier contient les fonctions li� � la gestion de la communications avec l'USART :
 * - Fonction definissant les E/S li� � l'uart.
 * - Initialisation de la dma pour l'envoi
 * - Initialisation de fa�on non bloquante de la reception en polling
 * - Gestions des commandes
 ******************************************************************************
 ******************************************************************************
 */
#include "system_dumby.h"
#include <stm32f10x.h>
#include <string.h>
#include <usart.h>

#include "cmdManager.h"


//#include "Battery.h"
//#include "motor.h"
//#include <math.h>
//#include "MAE.h"
//#include <stdlib.h>
//#include "gestionCmde.h"

#define TBuffer   30

/**
 * @brief		Variable global emission
 */
char sendString[TBuffer];

/**
 * @brief		Variable global reception
 */
char receiptString[TBuffer];

/**
 * @brief		Variable global nombre de caractère reçu.
 */
uint16_t cpt_Rx =0;
uint16_t i;

// test de k
uint16_t test;

// volatile temporary variable
volatile uint16_t tmp;
/** @addtogroup Projects
 * @{
 */

/** @addtogroup UART
 * @{
 */

/** @addtogroup Init_GPIO_IT_DMA_UART
 * @{
 */

/**
 * @brief Initialise l'UART avec les paramètres suivants : 9600 bauds / 1bits de stop / pas de partité ou de controle
 *
 * @param  Aucun
 * @retval Aucun
 */

void usartConfigure(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef Init_Structure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // Configure les lignes d'E/S
    // Configure Output ALTERNATE FONCTION PPULL PORT B6 Tx
    Init_Structure.GPIO_Pin = GPIO_Pin_6;
    Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
    Init_Structure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &Init_Structure);

    // Configure B7 Rx
    Init_Structure.GPIO_Pin = GPIO_Pin_7;
    Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &Init_Structure);

    // Configure l'USART
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);

    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    USART_Cmd(USART1, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // configure les interruptions de l'USART
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  Initialise la DMA de l'usart avec le buffer d'envoi sendString.
 *
 * @param  Aucun
 * @retval Aucun
 */
void usartInitDMA(uint16_t bufferSize)
{
    DMA_InitTypeDef DMA_InitStructure;

    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)sendString;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = bufferSize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel4, &DMA_InitStructure);
}

/**
 * @}
 */

/** @addtogroup Function Send - Receive
 * @{
 */

/**
 * @brief   Handler sur reception de l'uart.
 *
 * 			Reçoit les commandes UART sur Rx. Met à jour le buffer de reception
 * 			receiptString. Et appelle la fonction qui traitera la commande.
 * 			Commande finis sur un '\r'
 *
 * @attention Variable Global cpt_Rx
 * 			  taille reçu < 16.
 *
 * @param  Aucun
 * @retval Aucun
 */

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        receiptString[cpt_Rx] = USART_ReceiveData(USART1);
        if (cpt_Rx < 16)
            cpt_Rx++;

        if (receiptString[cpt_Rx - 1] == '\r') {
            cmdManage();

            if (Dumber.AddOn == FALSE) {
                cmdAddChecksum();
                usartSendData(); // Fonction bloquante
            }

            for (i = 0; i < cpt_Rx + 1; i++)
                receiptString[i] = 0;
            cpt_Rx = 0;
        }
    }

    // Code pour éviter les caractères fantômes
    tmp = USART1->SR;
    tmp = USART1->CR1;
    tmp = USART1->CR2;
    tmp = USART1->CR3;
    tmp = USART1->BRR;
    tmp = USART1->GTPR;
    tmp = USART1->SR;

    USART_ClearFlag(USART1, USART_FLAG_RXNE);
}

/**
 * @brief Chargement du buffer Tx dans la DMA et envoi via l'UART
 *
 * @param  Aucun
 * @retval Aucun
 */
void usartSendData(void){
    int taille;
    for(taille = 0; sendString[taille]!= '\r'; taille++);
    usartInitDMA(taille+1);
    DMA_Cmd(DMA1_Channel4, ENABLE);
    while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
    for(i=0; i<TBuffer;i++)
        sendString[i]=0;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
