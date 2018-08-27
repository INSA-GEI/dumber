#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include "cmsis_os.h"

typedef struct {
	char motorRSpeed;
	char motorLSpeed;
	int motorRDistance;
	int motorLDistance;
} Consigne;


extern char dumber_state;
extern char dumber_battery;
extern char dumber_wdActive;
extern int dumber_cptWd;
extern char dumber_activity;
extern Consigne dumber_consigne;

extern osMutexId cptWDHandle;
extern osMutexId wdEnableHandle;
extern osMutexId batteryLvlHandle;
extern osMutexId dumbyStateHandle;
extern osMutexId consigneHandle;
extern osMutexId busyHandle;
extern osMutexId activityCptHandle;

char getDumberState(void);
void setDumberState(char newState);

char getDumberBattery(void);
void setDumberBattery(char newBattery);

char getDumberWdActive(void);
void setDumberWdActive(char WdActive);

int getDumberCptWd(void);
void setDumberCptWd(char newCptWd);

char getDumberActivity(void);
void setDumberActivity(char activity);

Consigne getDumberConsigne(void);
void setDumberConsigne(Consigne *consigne);



#endif
