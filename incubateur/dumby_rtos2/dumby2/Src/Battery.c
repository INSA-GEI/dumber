#include "adc.h"
#include "Battery.h"
#include "common.h"

/*Global variable*/
/*Local Prototype*/

/*Local variable*/
uint16_t BAT_RAW[VOLTAGE_BUFFER_SIZE];

void BATTERY_INIT(void){
	int i;
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)BAT_RAW,VOLTAGE_BUFFER_SIZE);
	while(BAT_RAW[VOLTAGE_BUFFER_SIZE-1] == 0); // Attente de l'initialisation du buffer
	for(i=0;i<100000;i++); // Attente *calibration*
}

char BATTERY_FILTER(void){
	uint8_t i = 0;
	uint16_t batValue=0;
	for(i=0;i<VOLTAGE_BUFFER_SIZE;i++){
		batValue+= BAT_RAW[i] - 2048;
	}
	batValue = batValue/VOLTAGE_BUFFER_SIZE;
	batValue *=  0.097;
	return (char) batValue;
}

