#include "system_dumby.h"
#include <stm32f10x.h>
#include <string.h>
#include "cmde_usart.h"
#include "Battery.h"
#include "motor.h"
#include <math.h>
#include "MAE.h"
#include <stdlib.h>
#include "gestionCmde.h"

#define TBuffer   30

/** @Note
	* Ce fichier contient les fonctions lié à la gestion de la communications avec l'USART :
	* - Fonction definissant les E/S lié à l'usart.
	* - Initialisation de la dma pour l'envoi
	* - Initialisation de façon non bloquante de la reception en polling
	* - Gestions des commandes
	*/
char sendString[TBuffer];
char receiptString[TBuffer];
char message[TBuffer]; // Le traitement de l'emission se fera dans se tableau. On le transferera à la variable sendString au dernier moment de l'envoi.
uint16_t cpt_Rx =0;
uint16_t i;

// test de k
uint16_t test;



/**
	* @brief 			La fonction mapUsartPin va venir configurer le E/S du GPIO pour correspondre avec le schéma electrique en ressource.
	*							PB7 Analog Input / PB6 Alternate function output.
	*
	* @param  		Aucun
	* @retval 		Aucun
	*/
void MAP_UsartPin()
{
	GPIO_InitTypeDef Init_Structure;
	/// Configure Output ALTERNATE FONCTION PPULL PORT B6 Tx
	Init_Structure.GPIO_Pin = GPIO_Pin_6;
	Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
	Init_Structure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &Init_Structure);
	
	/// Configure B7 Rx
	Init_Structure.GPIO_Pin = GPIO_Pin_7;
  Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &Init_Structure);
}



/**
	* @brief 	Initialise la DMA de l'usart avec le buffer d'envoi sentString
	*	
	* @param  Aucun
	* @retval Aucun
	*/
void INIT_DMASend(void)
{
	DMA_InitTypeDef DMA_InitStructure;
  uint32_t i=0;
	
  DMA_DeInit(DMA1_Channel4);
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)sendString;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  //DMA_InitStructure.DMA_BufferSize =16;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  
	
	/* Recherche de la longueur de la chaine a envoyer */
	while ( sendString[i]!=0x0D) 
	{
		i=i+1;
	}
	
	if (i< TBuffer) DMA_InitStructure.DMA_BufferSize =i+1; // I+1 car on arrete la boucle au moment où l'on trouve CR. Mais il faut bien l'envoyer ...
	else DMA_InitStructure.DMA_BufferSize =0; // Hummm, ca semble moisi ce truc ! On a trouvé CR apres la fin du buffer alloué, la chaine n'a pas l'air bien formée
	
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
}

void INIT_IT_UsartReceive(void)
{
	   NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
	* @brief Initialise l'UART avec les paramétres suivants : 9600 bauds / 1bits de stop / pas de partité ou de controle
	*	
	* @param  Aucun
	* @retval Aucun
	*/

void INIT_USART(void)
{
		USART_InitTypeDef USART_InitStructure;
	
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
}

/**
	* @brief Recevoir les commandes UART sur Rx. Met à jour la position de la virgule et de la taille de la commande.
	*	
	* @param  Aucun
	* @varGlobal cpt_Rx, virgule
	* @retval Aucun
	*/

void USART1_IRQHandler(void)
{
	volatile uint16_t tmp;
	
	if(USART_GetITStatus(USART1,USART_IT_RXNE) !=RESET)
	{
		receiptString[cpt_Rx] = USART_ReceiveData(USART1);
		if(cpt_Rx<16)
			cpt_Rx++;
		
		if(receiptString[cpt_Rx-1]==13)
		{
			if(verifyCheckSum()==0)
			{
				manageCmd();
			}
			else
				strcpy(sendString,UNKNOW_ANS);
			
			if(Dumber.AddOn == FALSE)
			{
					inclusionCheckSum();
					sendDataUSART();
			}
			
			for( i = 0 ; i <cpt_Rx+1;i++)
				receiptString[i]=0;
			cpt_Rx=0;
		}
	}
	// Code pour éviter les caractéres fantôme
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
	* @brief Chargement du buffer Tx dans la DMA et envoi via l'USART
	*	
	* @param  Aucun
	* @retval Aucun
	*/
void sendDataUSART(void)
{
	INIT_DMASend();
	DMA_Cmd(DMA1_Channel4, ENABLE);
	while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET)
	{
	}
	for(i=0; i<TBuffer;i++)
		sendString[i]=0;
}
