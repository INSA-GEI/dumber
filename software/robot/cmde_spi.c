#include "system_dumby.h"
#include <stm32f10x.h>
#include <string.h>

#define TBufferSPI   16
SPI_InitTypeDef SPI_InitStructure;
uint16_t tailleT=0;
uint16_t tailleR=0;
char TSPIBuffer[TBufferSPI]={'S','A','L','U','T',0x0D};
char RSPIBuffer[TBufferSPI];

void SPI_INIT(void)
{
	//Init SPI
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 0;
  SPI_Init(SPI1, &SPI_InitStructure);
	
	 /* Enable SPI1 */
  SPI_Cmd(SPI1, ENABLE);
}


/*
		Le caractére 13 ou 0x0D ou retour chariot, ne peut être reçu ni envoyer.
*/
void SPI_SEND(void)
{
		if(TSPIBuffer[tailleT]!=0x0D)
		{
			while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
			SPI_I2S_SendData(SPI1, TSPIBuffer[tailleT]);
			tailleT++;
		}
		else
		{
			tailleT=0;
		}

}

void SPI_RECEP(void)
{		
		  if(SPI_I2S_ReceiveData(SPI1)!=0x0D)
			{
				while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
				RSPIBuffer[tailleR] = SPI_I2S_ReceiveData(SPI1);
				tailleR++;
			}
			else
			{
				tailleR=0;
			}
}
