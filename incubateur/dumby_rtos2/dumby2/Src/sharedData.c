#include "sharedData.h"
#include "common.h"

osMutexId cptWDHandle;
osMutexId wdEnableHandle;
osMutexId batteryLvlHandle;
osMutexId dumbyStateHandle;
osMutexId consigneHandle;
osMutexId busyHandle;
osMutexId activityCptHandle;


char dumber_state = IDLE;
char dumber_battery = 100;
char dumber_wdActive = WD_INACTIVE;
int dumber_cptWd= 0;
char dumber_activity = NOT_BUSY;
Consigne dumber_consigne;

char getDumberState(void){
	char result;
	osMutexWait(dumbyStateHandle,osWaitForever);
	result = dumber_state;
	osMutexRelease(dumbyStateHandle);
	return result;
}
void setDumberState(char newState){
	osMutexWait(dumbyStateHandle,osWaitForever);
	dumber_state=newState;
	osMutexRelease(dumbyStateHandle);
}

char getDumberBattery(void){
	char result;
	osMutexWait(batteryLvlHandle,osWaitForever);
	result = dumber_battery;
	osMutexRelease(batteryLvlHandle);
	return result;
}
void setDumberBattery(char newBattery){
	osMutexWait(batteryLvlHandle,osWaitForever);
	dumber_battery=newBattery;
	osMutexRelease(batteryLvlHandle);
}

char getDumberWdActive(void){
	char result;
	osMutexWait(wdEnableHandle,osWaitForever);
	result = dumber_wdActive;
	osMutexRelease(wdEnableHandle);
	return result;
}


void setDumberWdActive(char WdActive){
		osMutexWait(batteryLvlHandle,osWaitForever);
		dumber_wdActive=WdActive;
		osMutexRelease(batteryLvlHandle);
}

int getDumberCptWd(void){
	int result;
	osMutexWait(cptWDHandle,osWaitForever);
	result = dumber_cptWd;
	osMutexRelease(cptWDHandle);
	return result;
}
void setDumberCptWd(char newCptWd){
	osMutexWait(cptWDHandle,osWaitForever);
	dumber_cptWd = newCptWd;
	osMutexRelease(cptWDHandle);
}

char getDumberActivity(void){
	char result;
	osMutexWait(activityCptHandle,osWaitForever);
	result = dumber_activity;
	osMutexRelease(activityCptHandle);
	return result;
}
void setDumberActivity(char newActivity){
	osMutexWait(activityCptHandle,osWaitForever);
	dumber_activity = newActivity;
	osMutexRelease(activityCptHandle);
}

Consigne getDumberConsigne(void){
	Consigne cons;
	osMutexWait(consigneHandle,osWaitForever);
	cons = dumber_consigne;
	osMutexRelease(consigneHandle);
	return cons;
}

void setDumberConsigne(Consigne *consigne){
	osMutexWait(consigneHandle,osWaitForever);
	dumber_consigne = *consigne;
	osMutexRelease(consigneHandle);
}
