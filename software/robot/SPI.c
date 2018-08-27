#include "system_dumby.h"
#include <stm32f10x.h>
#include "SPI.h"


/**
	* @brief 	Initisalise les pin du GIO en entrée et sortie :
	*				 	PA5 : Clock généré par le maitre
	*	 			 	PA6	: Master Iutput Slave Onput (MISO)
	*					PA7	:	Master Output Slave Input (MOSI)
	*/

void MAP_pinSpi(void)
{
	GPIO_InitTypeDef Init_Structure;
	
	Init_Structure.GPIO_Pin = GPIO_Pin_5;
	Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	Init_Structure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &Init_Structure);
	
	Init_Structure.GPIO_Pin = GPIO_Pin_6;
  Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &Init_Structure);
	
	
	Init_Structure.GPIO_Pin = GPIO_Pin_7;
  Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &Init_Structure);
	
	Init_Structure.GPIO_Pin = GPIO_Pin_13;
	Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	Init_Structure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &Init_Structure);
	
	
	// A suprimmer aprés le test
	
	Init_Structure.GPIO_Pin = GPIO_Pin_13;
	Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	Init_Structure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &Init_Structure);
	
	Init_Structure.GPIO_Pin = GPIO_Pin_14;
  Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &Init_Structure);
		
	
	Init_Structure.GPIO_Pin = GPIO_Pin_15;
  Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &Init_Structure);
	

	
	
}



/**
	* @brief 	Configure le periphérique SPI. FullDuplex et Slave Mode;
	* 				Pour tester le SPI on testera les envoi et les recepetions avec l'aide du SPI2.
	*					Il sera donc initialiser. Et reboucler.
	*					
	*/
void INIT_SPI(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
	
	
	// A suprimmer aprés le test
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(SPI2, &SPI_InitStructure);
}

/**
	*	@brief Configure le Rx de l'esclave (Stm32)
	* 			 
	*/

void INIT_IT_SPI(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

  /* 1 bit for pre-emption priority, 3 bits for subpriority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Configure and enable SPI_SLAVE interrupt --------------------------------*/
  NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_Init(&NVIC_InitStructure);
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);

}
