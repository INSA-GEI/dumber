#ifndef GestionCmde_H
#define GestionCmde_H

#include <stm32f10x.h>

void manageCmd(void);
char verifyCheckSum(void);
void inclusionCheckSum(void);
void actionReset(void);
void actionBusyState(void);
void actionPing(void);
void actionVersion(void);
void actionStartWWD(void);
void actionMove(void);
void actionTurn(void);
void actionBatteryVoltage(void);
void actionStartWithWD(void);
void actionResetWD(void);
void actionDebug(void);
#endif /* gestionCmde.h */

