#include "serviceCom.h"
#include "usart.h"


// pour test seulement
#include "xbee.h"
osSemaphoreId synchroMesHandle;

char RxBuffer[TRAME_SIZE_RECEIPT];

void comInit(){
	HAL_UART_Receive_DMA(&huart1,(uint8_t*)RxBuffer,TRAME_SIZE_RECEIPT);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	//xbeeSend(0x10,"OKL");
	osSemaphoreRelease(synchroMesHandle);
}

